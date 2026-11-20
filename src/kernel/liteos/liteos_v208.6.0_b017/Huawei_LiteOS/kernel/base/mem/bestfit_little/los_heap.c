/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2021. All rights reserved.
 * Description: LiteOS Mem Module Implementation
 * Author: Huawei LiteOS Team
 * Create: 2013-05-12
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --------------------------------------------------------------------------- */

#include "los_memory_pri.h"
#include "los_memory_internal.h"

#include "string.h"
#include "securec.h"
#include "los_hwi.h"
#include "los_config.h"
#include "los_typedef.h"
#include "los_task_pri.h"
#include "los_exc.h"

#ifdef LOSCFG_KERNEL_LMS
#include "los_lms_pri.h"
#endif

#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
UINTPTR g_callerRA[TASK_NUM];
#endif

#define HEAP_CAST(t, exp)       ((t)(exp))
#define HEAP_ALIGN              sizeof(UINTPTR)
#define MALLOC_MAXSIZE          (0xFFFFFFFF - HEAP_ALIGN + 1)
#define NODE_UNUSED             0
#define NODE_USED               1

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
STATIC INLINE BOOL OsHeapIsSmallNode(UINT32 data_size)
{
    return (data_size < LOS_MEM_SMALL_NODE_THRESHOLD) ? TRUE : FALSE;
}

STATIC INLINE UINT32 OsHeapNodeSize(UINT32 data_size)
{
    return (data_size < LOS_MEM_SMALL_NODE_THRESHOLD) ? sizeof(struct LosHeapNodeSmall)  : sizeof(struct LosHeapNodeLarge);
}
#else
#define OsHeapIsSmallNode(size)  FALSE
#endif


/*
 * Description : look up the next memory node according to one memory node in the memory block list.
 * Input       : struct LosHeapManager *heapMan --- Pointer to the manager,to distinguish heap
 *               struct LosHeapNode *node --- Size of memory in bytes to allocate
 * Return      : Pointer to next memory node
 */
LosHeapNode *OsHeapPrvGetNext(const struct LosHeapManager *heapMan, const LosHeapNode *node)
{
    return (heapMan->tail == node) ? NULL : (LosHeapNode *)(UINTPTR)(NODE_DATA(node) + NODE_SIZE(node));
}

#ifdef LOSCFG_MEM_TASK_STAT
STATIC INLINE VOID OsHeapStatInit(struct LosHeapManager *heapMan, UINT32 size)
{
    (VOID)size;
    (VOID)memset(&heapMan->stat, 0, sizeof(Memstat));

    heapMan->stat.memTotalUsed = OS_MEM_NODE_HEAD_SIZE + (UINT32)sizeof(struct LosHeapManager);
    heapMan->stat.memTotalPeak = heapMan->stat.memTotalUsed;
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
    (VOID)memset(g_callerRA, 0x0, sizeof(g_callerRA));
#endif
}

STATIC UINT32 OsGetCurTaskId(VOID)
{
    UINT32 taskId;

    if ((OsCurrTaskGet() != NULL) && OS_INT_INACTIVE) {
        /*
         * after OsTaskInit, OsCurrTaskGet() is not null, but id is the same
         * as (LOSCFG_BASE_CORE_TSK_LIMIT + 1), so it will be recorded into
         * the last one of the array.
         */
        taskId = LOS_CurTaskIDGet();
    } else {
        taskId = TASK_NUM - 1;
    }
    return taskId;
}

STATIC INLINE VOID OsHeapStatAddUsed(struct LosHeapManager *heapMan, LosHeapNode *node)
{
    UINT32 blockSize = NODE_HEAD_SIZE(node) + NODE_SIZE(node);

    SET_NODE_TASKID(node, OsGetCurTaskId());
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
    SET_NODE_CALLER_RA(node, g_callerRA[NODE_TASKID(node)]);
    g_callerRA[NODE_TASKID(node)] = (UINTPTR)0;
#endif
    OS_MEM_ADD_USED(&heapMan->stat, blockSize, NODE_TASKID(node));
}

STATIC INLINE VOID OsHeapStatDecUsed(struct LosHeapManager *heapMan, LosHeapNode *node)
{
    UINT32 taskId = NODE_TASKID(node);
    UINT32 blockSize = NODE_HEAD_SIZE(node) + NODE_SIZE(node);

    OS_MEM_REDUCE_USED(&heapMan->stat, blockSize, taskId);
}

#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
VOID OsSaveCallerRa(UINTPTR callerRa)
{
    UINT32 taskId =  OsGetCurTaskId();
    g_callerRA[taskId] = callerRa;
}

BOOL OsIsSetCallerRa(VOID)
{
    UINT32 taskId =  OsGetCurTaskId();
    return (g_callerRA[taskId] != (UINTPTR)0);
}

#endif

#else /* LOSCFG_MEM_TASK_STAT */

STATIC INLINE VOID OsHeapStatInit(struct LosHeapManager *heapMan, UINT32 size)
{
    (VOID)heapMan;
    (VOID)size;
}

STATIC INLINE VOID OsHeapStatAddUsed(struct LosHeapManager *heapMan, LosHeapNode *node)
{
    (VOID)heapMan;
    (VOID)node;
}

STATIC INLINE VOID OsHeapStatDecUsed(struct LosHeapManager *heapMan, LosHeapNode *node)
{
    (VOID)heapMan;
    (VOID)node;
}
#endif /* LOSCFG_MEM_TASK_STAT */

#ifdef LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK

UINT32 OsHeapIntegrityCheck(struct LosHeapManager *heap)
{
    LosHeapNode *node = (LosHeapNode *)(heap + 1);
    UINTPTR heapStart = (UINTPTR)heap;
    UINTPTR heapEnd = (UINTPTR)node + heap->size;

    while (node != NULL) {
        if ((UINTPTR)node < heapStart || (UINTPTR)node > heapEnd) {
            LOS_PANIC("node %p has been corrupted.\n", node);
            return LOS_NOK;
        }

        node = OsHeapPrvGetNext(heap, node);
    }

    return LOS_OK;
}

#else /* LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK */

UINT32 OsHeapIntegrityCheck(struct LosHeapManager *heap)
{
    (VOID)heap;
    return LOS_OK;
}

#endif /* LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK */

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION

VOID *OsMemAlloc(VOID *pool, UINT32 size)
{
    return OsHeapAlloc(pool, size);
}

UINT32 OsMemFree(VOID *pool, const VOID *ptr)
{
    if (OsHeapFree(pool, ptr) == TRUE) {
        return LOS_OK;
    } else {
        return LOS_NOK;
    }
}

#endif /* LOSCFG_KERNEL_MEM_SLAB_EXTENTION */

#ifdef LOSCFG_KERNEL_LMS
STATIC INLINE VOID OsLmsFirstNodeMark(VOID *pool, LosHeapNode *node)
{
    if (g_lms == NULL) {
        return;
    }
    g_lms->simpleMark((UINTPTR)pool, (UINTPTR)node, LMS_SHADOW_PAINT_U8);
    g_lms->simpleMark((UINTPTR)node, (UINTPTR)node + NODE_HEAD_SIZE(node), LMS_SHADOW_REDZONE_U8);
    g_lms->simpleMark((UINTPTR)node + NODE_HEAD_SIZE(node), (UINTPTR)(NODE_DATA(node) + NODE_SIZE(node)),
        LMS_SHADOW_AFTERFREE_U8);
}

STATIC INLINE VOID OsLmsAllocAlignMark(VOID *ptr, VOID *alignedPtr, UINT32 size)
{
    LosHeapNode *node = NULL;
    UINTPTR nextAddr;
    if (g_lms == NULL) {
        return;
    }
    node = ((LosHeapNode *)ptr) - 1;
    nextAddr = (UINTPTR)(NODE_DATA(node) + NODE_SIZE(node));

    if (ptr != alignedPtr) {
        g_lms->simpleMark((UINTPTR)ptr, (UINTPTR)ptr + sizeof(UINT32), LMS_SHADOW_PAINT_U8);
        g_lms->simpleMark((UINTPTR)ptr + sizeof(UINT32), (UINTPTR)alignedPtr, LMS_SHADOW_REDZONE_U8);
    }

    /* mark remind as redzone */
    g_lms->simpleMark(LMS_ADDR_ALIGN((UINTPTR)alignedPtr + size), nextAddr, LMS_SHADOW_REDZONE_U8);
}
#endif

/*
 * Description : To initialize the heap memory and get the begin address and size of heap memory,
 *               then initialize LosHeapManager.
 * Input       : VOID *pool  --- begin address of the heap memory pool
 *               UINT32 size --- size of the heap memory pool
 * Return      : 1:success 0:error
 */
BOOL OsHeapInit(VOID *pool, UINT32 size)
{
#ifdef LOSCFG_RECLAIM_STARTUP_STACK
    LosHeapNode *reservNode = NULL;
#endif
    LosHeapNode *node = NULL;
    struct LosHeapManager *heapMan = HEAP_CAST(struct LosHeapManager *, pool);

    if (heapMan == NULL) {
        return FALSE;
    }
#ifdef LOSCFG_KERNEL_LMS
    UINT32 resize = 0;
    UINT32 poolSize = size;
    if (g_lms != NULL) {
        /*
         * resize == 0, shadow memory init failed, no shadow memory for this pool, set poolSize as original size.
         * resize != 0, shadow memory init successful, set poolSize as resize.
         */
        resize = g_lms->init(pool, size);
        size = (resize == 0) ? poolSize : resize;
        if (size < OS_MEM_MIN_POOL_SIZE) {
            return LOS_NOK;
        }
    }
#endif
#ifdef LOSCFG_RECLAIM_STARTUP_STACK
    if (pool == OS_SYS_MEM_ADDR) {
        (VOID)memset(pool, 0, size - SYS_MEM_RESERVE_STARTUP_STACK_SIZE);    // skip reserved startup stack
    } else
#endif
    {
        (VOID)memset(pool, 0, size);
    }

    heapMan->size = size - (UINT32)sizeof(struct LosHeapManager);

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
    heapMan->heapBaseAddr = (UINTPTR)pool;
#endif

    node = heapMan->head = (LosHeapNode *)((UINT8*)pool + sizeof(struct LosHeapManager));

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
    SET_NODE_TYPE(node, OsHeapIsSmallNode(heapMan->size) ? LOS_MEM_NODE_TYPE_SMALL : LOS_MEM_NODE_TYPE_LARGE);
    SET_NODE_PREV(heapMan, node, NULL);
    SET_NODE_USED_STAT(node, NODE_UNUSED);
#endif

#ifdef LOSCFG_RECLAIM_STARTUP_STACK
    if (pool == OS_SYS_MEM_ADDR) {
        SET_NODE_SIZE(node, heapMan->size - SYS_MEM_RESERVE_STARTUP_STACK_SIZE - \
                     OS_MEM_NODE_HEAD_SIZE_TWO);
        reservNode = (LosHeapNode*)(UINTPTR)((UINT8*)pool + size - \
                     SYS_MEM_RESERVE_STARTUP_STACK_SIZE - OS_MEM_NODE_HEAD_SIZE);
        SET_NODE_SIZE(reservNode, SYS_MEM_RESERVE_STARTUP_STACK_SIZE);
        if (OsHeapIsSmallNode(SYS_MEM_RESERVE_STARTUP_STACK_SIZE)) {
            SET_NODE_TYPE(reservNode, LOS_MEM_NODE_TYPE_SMALL);
        } else {
            SET_NODE_TYPE(reservNode, LOS_MEM_NODE_TYPE_LARGE);
        }
        SET_NODE_ALIGN(reservNode, 0);
        SET_NODE_USED_STAT(reservNode, NODE_USED);
        SET_NODE_PREV(heapMan, reservNode, node);

        heapMan->tail = reservNode;
    } else
#endif
    {
        SET_NODE_SIZE(node, heapMan->size - NODE_HEAD_SIZE(node));
        heapMan->tail = node;
    }

    OsHeapStatInit(heapMan, size);
#ifdef LOSCFG_RECLAIM_STARTUP_STACK
    if (pool == OS_SYS_MEM_ADDR) {
        OsHeapStatAddUsed(heapMan, reservNode);
    }
#endif
#ifdef LOSCFG_KERNEL_LMS
    if (resize != 0) {
        OsLmsFirstNodeMark(pool, node);
    }
#endif
    return TRUE;
}

STATIC INLINE VOID OsHeapSplitNode(LosHeapNode *best, struct LosHeapManager *heapMan, UINT32 alignSize)
{
    LosHeapNode *node = NULL;
    LosHeapNode *next = NULL;

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
    UINT32 totalSize = NODE_HEAD_SIZE(best) + NODE_SIZE(best);
    UINT32 bestTotalSize = alignSize + OsHeapNodeSize(alignSize);
    UINT32 nodeTotalSize = totalSize - bestTotalSize;
    node = (LosHeapNode*)(UINTPTR)((UINT8*)best + alignSize + OsHeapNodeSize(alignSize));
#else
    /* hole divide into 2 */
    node = (LosHeapNode*)(UINTPTR)(NODE_DATA(best) + alignSize);
    SET_NODE_USED_STAT(node, NODE_UNUSED);
#endif

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
    UINT32 remainedSize = nodeTotalSize - sizeof(struct LosHeapNodeSmall);
    if (OsHeapIsSmallNode(remainedSize)) {
        SET_NODE_TYPE(node, LOS_MEM_NODE_TYPE_SMALL);
    } else {
        SET_NODE_TYPE(node, LOS_MEM_NODE_TYPE_LARGE);
    }
    SET_NODE_SIZE(node, nodeTotalSize - OsHeapNodeSize(remainedSize));
    SET_NODE_USED_STAT(node, NODE_UNUSED);
    LosHeapNode *prev = NODE_PREV(heapMan, best);
    if (OsHeapIsSmallNode(alignSize)) {
        SET_NODE_TYPE(best, LOS_MEM_NODE_TYPE_SMALL);
    } else {
        SET_NODE_TYPE(best, LOS_MEM_NODE_TYPE_LARGE);
    }
    SET_NODE_PREV(heapMan, best, prev);
    SET_NODE_USED_STAT(best, NODE_UNUSED);
    SET_NODE_SIZE(best, alignSize);
#else
    SET_NODE_SIZE(node, NODE_SIZE(best) - alignSize - NODE_HEAD_SIZE(node));
    SET_NODE_SIZE(best, alignSize);
#endif
    SET_NODE_PREV(heapMan, node, best);
    if (best != heapMan->tail) {
        next = OsHeapPrvGetNext(heapMan, node);
        if (next != NULL) {
            SET_NODE_PREV(heapMan, next, node);
        }
    } else {
        heapMan->tail = node;
    }
}

#ifdef LOSCFG_MEM_TASK_STAT
STATIC UINT32 OsHeapInfoDump(VOID *pool)
{
    LosHeapStatus heapStat;
    UINT32 err;

    err = OsHeapStatisticsGet(pool, &heapStat);
    if (err != LOS_OK) {
        return LOS_NOK;
    }
    PRINT_ERR("[Heap@0x%x stat] total:0x%x, used:0x%x, free:0x%x, peak usage:0x%x, peak free:0x%x\r\n", \
              (UINTPTR)pool, \
              (uint32_t)(heapStat.totalUsedSize + heapStat.totalFreeSize), \
              (uint32_t)(heapStat.totalUsedSize), (uint32_t)(heapStat.totalFreeSize), \
              (uint32_t)(heapStat.usageWaterLine), \
              (uint32_t)(heapStat.totalFreeSize + heapStat.totalUsedSize - heapStat.usageWaterLine));
    return LOS_OK;
}
#endif
/*
 * Description : To alloc memory block from the heap memory poll
 * Input       : VOID *pool --- Pointer to the manager,to distinguish heap
 *               UINT32 size --- size of the heap memory pool
 * Return      : NULL:error, other value:the address of the memory we alloced
 */
VOID *OsHeapAlloc(VOID *pool, UINT32 size)
{
    LosHeapNode *node = NULL;
    LosHeapNode *best = NULL;
    VOID *ptr = NULL;
    UINT32 alignSize = ALIGN(size, HEAP_ALIGN);

    struct LosHeapManager *heapMan = HEAP_CAST(struct LosHeapManager *, pool);
    if ((heapMan == NULL) || (size > MALLOC_MAXSIZE)) {
        return NULL;
    }

    if (OsHeapIntegrityCheck(heapMan) != LOS_OK) {
        return NULL;
    }

    node = heapMan->tail;
    UINT32 bestSz = 0;
    while (node != NULL) {
        if ((NODE_USED_STAT(node) == NODE_UNUSED) && (NODE_SIZE(node) >= alignSize) &&
            ((best == NULL) || bestSz > NODE_SIZE(node))) {
            best = node;
            bestSz = NODE_SIZE(node);
            if (bestSz == alignSize) {
                goto SIZE_MATCH;
            }
        }
        node = NODE_PREV(heapMan, node);
    }

    /* alloc failed */
    if (best == NULL) {
        PRINT_ERR("No mem to alloc 0x%x Bytes at pool_addr=0x%x!\n", alignSize, (UINTPTR)pool);
#ifdef LOSCFG_MEM_TASK_STAT
       OsHeapInfoDump(pool);
#endif
        goto OUT;
    }

    if ((NODE_SIZE(best) - alignSize) > OS_MEM_NODE_HEAD_SIZE) {
        OsHeapSplitNode(best, heapMan, alignSize);
    }

SIZE_MATCH:
    SET_NODE_ALIGN(best, 0);
    SET_NODE_USED_STAT(best, NODE_USED);
    ptr = NODE_DATA(best);

    OsHeapStatAddUsed(heapMan, best);

#ifdef LOSCFG_KERNEL_LMS
    if (g_lms != NULL) {
        LosHeapNode *next = (LosHeapNode *)(UINTPTR)(NODE_DATA(best) + NODE_SIZE(best));
        g_lms->mallocMark(best, next, NODE_HEAD_SIZE(best));
    }
#endif
OUT:
    return ptr;
}

/*
 * Description : To alloc memory block from the heap memory poll with
 * Input       : VOID *pool   --- Pointer to the manager,to distinguish heap
 *               UINT32 size   --- size of the heap memory pool
 *               UINT32 boundary --- boundary the heap needs align
 * Return      : NULL:error, other value:the address of the memory we alloced
 */
VOID* OsHeapAllocAlign(VOID *pool, UINT32 size, UINT32 boundary)
{
    UINT32 useSize;
    UINT32 gapSize;
    VOID *ptr = NULL;
    VOID *alignedPtr = NULL;

    if ((pool == NULL) || (size == 0) || (boundary < sizeof(VOID *)) || !IS_ALIGNED(boundary, boundary)) {
        return NULL;
    }

    /* worst case is that the node happen to be 4 bytes ahead of the boundary */
    useSize = (size + boundary) - (UINT32)sizeof(VOID*);
    if (useSize < size) {
        return NULL;
    }

    ptr = OsHeapAlloc(pool, useSize);
    if (ptr != NULL) {
        alignedPtr = (VOID *)(UINTPTR)OS_MEM_ALIGN(ptr, boundary);
        if (alignedPtr == ptr) {
#ifdef LOSCFG_KERNEL_LMS
            OsLmsAllocAlignMark(ptr, alignedPtr, size);
#endif
            goto OUT;
        }

        gapSize = (UINTPTR)alignedPtr - (UINTPTR)ptr;
        OS_MEM_SET_ALIGN_FLAG(gapSize);
        *((UINT32 *)((UINTPTR)alignedPtr - sizeof(UINTPTR))) = gapSize;

#ifdef LOSCFG_KERNEL_LMS
        OsLmsAllocAlignMark(ptr, alignedPtr, size);
#endif
        ptr = alignedPtr;
    }
OUT:
    return ptr;
}

STATIC VOID OsHeapDoFree(struct LosHeapManager *heapMan, LosHeapNode *curNode)
{
    LosHeapNode *node = curNode;
    LosHeapNode *next = NULL;
    SET_NODE_USED_STAT(node, NODE_UNUSED);
#ifdef LOSCFG_KERNEL_LMS
    LosHeapNode *nextBackup = OsHeapPrvGetNext(heapMan, node);
    UINT32 curNodeSize = NODE_SIZE(curNode);
#endif
    while ((NODE_PREV(heapMan, node)) && (!NODE_USED_STAT(NODE_PREV(heapMan, node)))) {
        node = NODE_PREV(heapMan, node);
    }

    next = OsHeapPrvGetNext(heapMan, node);
    while (next != NULL) {
        if (NODE_USED_STAT(next)) {
            SET_NODE_PREV(heapMan, next, node);
            break;
        }
#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
        UINT32 newSize = NODE_SIZE(node) + NODE_HEAD_SIZE(next) + NODE_SIZE(next);
        if (OsHeapIsSmallNode(newSize) == FALSE && NODE_TYPE(node) == LOS_MEM_NODE_TYPE_SMALL) {
            LosHeapNode *prev = NODE_PREV(heapMan, node);
            SET_NODE_TYPE(node, LOS_MEM_NODE_TYPE_LARGE);
            SET_NODE_PREV(heapMan, node, prev);
            SET_NODE_USED_STAT(node, NODE_UNUSED);
            SET_NODE_SIZE(node, newSize - sizeof(struct LosHeapNodeSmall));
        } else {
            SET_NODE_SIZE(node, newSize);
        }
#else
        NODE_SIZE(node) += (OS_MEM_NODE_HEAD_SIZE + next->size);
#endif
        if (heapMan->tail == next) {
            heapMan->tail = node;
        }
        next = OsHeapPrvGetNext(heapMan, node);
    }

#ifdef LOSCFG_KERNEL_LMS
    if (g_lms != NULL) {
        OsHeapLmsCleanup(heapMan, curNode, node, nextBackup, curNodeSize);
    }
#endif
}

#ifdef LOSCFG_KERNEL_LMS
STATIC VOID OsHeapLmsCleanup(struct LosHeapManager *heapMan, LosHeapNode *curNode,
                             LosHeapNode *node, LosHeapNode *nextBackup, UINT32 curNodeSize)
{
    if (curNode != node) {
        (VOID)memset_s(curNode, NODE_HEAD_SIZE(curNode), 0, NODE_HEAD_SIZE(curNode));
    }
    LosHeapNode *nextNode = OsHeapPrvGetNext(heapMan, node);
    if (nextBackup != nextNode) {
        (VOID)memset_s(nextBackup, NODE_HEAD_SIZE(nextBackup), 0, NODE_HEAD_SIZE(nextBackup));
    }
    nextBackup = (LosHeapNode *)((UINTPTR)curNode + NODE_HEAD_SIZE(curNode) + curNodeSize);
    g_lms->freeMark(curNode, nextBackup, NODE_HEAD_SIZE(curNode));
}
#endif

/*
 * Description : To free the  memory block from  heap memory poll
 * Input       : VOID* pool --- Pointer to the manager,to distinguish heap
 *               VOID* ptr --- the pointer of heap memory we want to free
 * Return      : 1:success 0:error
 */
BOOL OsHeapFree(VOID *pool, const VOID *ptr)
{
    LosHeapNode *node = NULL;

    UINT32 gapSize;
    BOOL ret = TRUE;

    struct LosHeapManager *heapMan = HEAP_CAST(struct LosHeapManager *, pool);
    if ((heapMan == NULL) || (ptr == NULL)) {
        return LOS_NOK;
    }

    /* find the real ptr through gap size */
    gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(UINTPTR)));
    if (OS_MEM_GET_ALIGN_FLAG(gapSize)) {
        gapSize = OS_MEM_GET_ALIGN_GAPSIZE(gapSize);
        ptr = (VOID *)((UINTPTR)ptr - gapSize);
    }

    if (((UINTPTR)ptr < (UINTPTR)heapMan->head) ||
        ((UINTPTR)ptr > ((UINTPTR)heapMan->tail + OS_MEM_NODE_HEAD_SIZE))) {
        PRINT_ERR("0x%lx out of range!\n", (UINTPTR)ptr);
        return FALSE;
    }

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
    node = (LosHeapNode *)((UINT8 *)ptr - sizeof(struct LosHeapNodeSmall));
    if (((node != heapMan->tail) && NODE_PREV(heapMan, node) != NULL &&
         (UINTPTR)OsHeapPrvGetNext(heapMan, NODE_PREV(heapMan, node)) != (UINTPTR)node) ||
        ((node != heapMan->head) && (NODE_PREV(heapMan, node) == NULL))) {
        node = (LosHeapNode *)((UINT8 *)ptr - sizeof(struct LosHeapNodeLarge));
    }
#else
    node = ((LosHeapNode *)ptr) - 1;
#endif
LosHeapNode *prevNode = NODE_PREV(heapMan, node);
    /* check if the address is a node of the heap memory list */
    if (((NODE_USED_STAT(node) == NODE_UNUSED) || (node != heapMan->head &&
        ((prevNode == NULL) || ((UINTPTR)prevNode < (UINTPTR)heapMan->head) ||
        ((UINTPTR)prevNode > ((UINTPTR)heapMan->tail + NODE_HEAD_SIZE(node)) ||
        ((UINTPTR)OsHeapPrvGetNext(heapMan, prevNode) != (UINTPTR)node)))))) {
        goto OUT;
    }

    OsHeapStatDecUsed(heapMan, node);
    OsHeapDoFree(heapMan, node);

OUT:
    return ret;
}

/*
 * Description : collect heap statistics
 * Input       : pool --- Pointer to the manager, to distinguish heap
 * Output      : status --- heap statistics
 * Return      : LOS_OK on success or error code on failure
 */
UINT32 OsHeapStatisticsGet(VOID *pool, LosHeapStatus *status)
{
    UINT32 heapUsed = 0;
    UINT32 maxFreeNodeSize = 0;
    UINT32 freeNodeNum = 0;
    UINT32 usedNodeNum = 0;

    LosHeapNode *node = NULL;
    struct LosHeapManager *ramHeap = HEAP_CAST(struct LosHeapManager *, pool);

    if (ramHeap == NULL) {
        return LOS_NOK;
    }

    if (status == NULL) {
        return LOS_NOK;
    }

    /* heap manager header use heap space */
    heapUsed += (UINT32)sizeof(struct LosHeapManager);

    node = ramHeap->tail;
    while (node != NULL) {
        if (NODE_USED_STAT(node)) {
            heapUsed += (NODE_SIZE(node) + NODE_HEAD_SIZE(node));
            usedNodeNum++;
        } else {
            if (NODE_SIZE(node) > maxFreeNodeSize) {
                maxFreeNodeSize = NODE_SIZE(node);
            }
            freeNodeNum++;
        }
        node = NODE_PREV(ramHeap, node);
    }

    if (ramHeap->size < heapUsed) {
        return LOS_NOK;
    }

    status->totalUsedSize = heapUsed;
    status->maxFreeNodeSize = maxFreeNodeSize;
    status->totalFreeSize = ramHeap->size - status->totalUsedSize;
    status->usedNodeNum = usedNodeNum;
    status->freeNodeNum = freeNodeNum;

#ifdef LOSCFG_MEM_TASK_STAT
    status->usageWaterLine = ramHeap->stat.memTotalPeak;
#endif

    return LOS_OK;
}

/*
 * Description : get max free block size
 * Input       : pool --- Pointer to the manager, to distinguish heap
 * Return      : max free block size
 */
UINT32 OsHeapGetMaxFreeBlkSize(VOID *pool)
{
    UINT32 size = 0;
    UINT32 temp;
    LosHeapNode *node = NULL;
    struct LosHeapManager *ramHeap = HEAP_CAST(struct LosHeapManager *, pool);

    if (ramHeap == NULL) {
        return LOS_NOK;
    }

    node = ramHeap->tail;
    while (node != NULL) {
        if (!(NODE_USED_STAT(node))) {
            temp = NODE_SIZE(node);
            if (temp > size) {
                size = temp;
            }
        }
        node = NODE_PREV(ramHeap, node);
    }
    return size;
}

UINTPTR OsHeapLastUsedGet(VOID *pool)
{
    LosHeapNode *node = NULL;
    struct LosHeapManager *heapMan = HEAP_CAST(struct LosHeapManager *, pool);
    if (heapMan == NULL) {
        return LOS_NOK;
    }

    if (OsHeapIntegrityCheck(heapMan) != LOS_OK) {
        return LOS_NOK;
    }

    node = heapMan->tail;
    if (NODE_USED_STAT(node) == NODE_UNUSED) {
        return (UINTPTR)((CHAR *)node + NODE_HEAD_SIZE(node));
    } else {
        return (UINTPTR)((CHAR*)node + NODE_HEAD_SIZE(node) + NODE_SIZE(node));
    }
}

#ifdef LOSCFG_MEM_TASK_STAT
UINT32 OsHeapTaskHeapInfoGet(VOID *pool, UINT8 taskId, VOID (*print)(const char *fmt, ...))
{
    UINT32 totalUsedSize = 0;
    UINT32 idx = 0;
    UINTPTR callerRa = 0;
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskId);

    LosHeapNode *node = NULL;
    struct LosHeapManager *ramHeap = HEAP_CAST(struct LosHeapManager *, pool);

    if (ramHeap == NULL || taskId > TASK_NUM - 1) {
        return LOS_NOK;
    }

    if (taskId == TASK_NUM - 1) {
        print("Other malloc details:\r\n");
    } else {
        print("Task:%s(taskId=%02d) malloc details:\r\n", taskCB->taskName, taskId);
    }
    node = ramHeap->tail;
    while (node != NULL) {
        if (NODE_TASKID(node) == taskId && NODE_USED_STAT(node)) {
            totalUsedSize += NODE_SIZE(node) + OS_MEM_NODE_HEAD_SIZE;
            idx++;
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
            callerRa = NODE_CALLER_RA(node);
#endif
            print("mem[%03d]: size=%08d Bytes, addr=0x%08x, callerRA=0x%08x\r\n", idx, \
                  (NODE_SIZE(node) + OS_MEM_NODE_HEAD_SIZE), (UINTPTR)NODE_DATA(node), callerRa);
        }
        node = NODE_PREV(ramHeap, node);
    }
    print("Current total alloc size=%d Bytes\r\n", totalUsedSize);

    return LOS_OK;
}
#endif
