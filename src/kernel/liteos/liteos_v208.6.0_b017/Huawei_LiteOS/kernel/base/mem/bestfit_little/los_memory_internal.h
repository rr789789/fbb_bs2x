/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2020. All rights reserved.
 * Description: LiteOS memory Module Implementation
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

/*
 * @defgroup los_heap Heap
 * @ingroup kernel
 */

#ifndef _LOS_MEMORY_INTERNAL_H
#define _LOS_MEMORY_INTERNAL_H

#include "los_typedef.h"
#include "los_memory_pri.h"

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
#include "los_heap_hook.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
struct LosHeapNodeSmall {
    UINT32 node_type   : 1;
    UINT32 taskId      : 4;
    UINT32 used        : 1;
    UINT32 align       : 1;
    UINT32 size        : 9;
    UINT32 prev_offset : 16;
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
    UINTPTR callerRa;
#endif
    UINT8  data[0];
};
struct LosHeapNodeLarge {
    UINT32 node_type   : 1;
    UINT32 taskId      : 4;
    UINT32 used        : 1;
    UINT32 align       : 1;
    UINT32 size        : 21;
    struct LosHeapNodeLarge *prev;
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
    UINTPTR callerRa;
#endif
    UINT8  data[0];
};
typedef union LosHeapNode {
    struct LosHeapNodeSmall small;
    struct LosHeapNodeLarge large;
} LosHeapNode;
#define LOS_MEM_SMALL_NODE_THRESHOLD    512U
#define LOS_MEM_NODE_TYPE_SMALL         0U
#define LOS_MEM_NODE_TYPE_LARGE         1U
#define LOS_MEM_SMALL_NODE_SIZE_MAX     0x1FFU
#define LOS_MEM_PREV_OFFSET_MASK        0xFFFFU
#else
#if defined(LOSCFG_FAMILY_AIOT) && defined(LOSCFG_MEM_TASK_STAT)
typedef struct LosHeapNode {
    struct LosHeapNode *prev;
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
    UINTPTR callerRa;
#endif
    UINT32 size     : 22;
    UINT32 taskId   : 8;
    UINT32 used     : 1;
    UINT32 align    : 1;
    UINT8  data[0];
}LosHeapNode;
#else
typedef struct LosHeapNode {
    struct LosHeapNode *prev;
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 taskId;
#elif defined(LOSCFG_AARCH64)
    UINT32 res;     /* For alignment */
#endif
    UINT32 size     : 30;
    UINT32 used     : 1;
    UINT32 align    : 1;
    UINT8  data[0];
}LosHeapNode;
#endif
#endif

#define OS_MEM_ALIGN_FLAG                     0x80000000
#define OS_MEM_SET_ALIGN_FLAG(align)          ((align) = ((align) | OS_MEM_ALIGN_FLAG))
#define OS_MEM_GET_ALIGN_FLAG(align)          ((align) & OS_MEM_ALIGN_FLAG)
#define OS_MEM_GET_ALIGN_GAPSIZE(align)       ((align) & (~OS_MEM_ALIGN_FLAG))
#define OS_MEM_NODE_HEAD_SIZE                 (UINT32)sizeof(LosHeapNode)
/* Requires at least 1 byte of allocated space */
#ifdef LOSCFG_RECLAIM_STARTUP_STACK
#define OS_SYS_MEM_MIN_POOL_SIZE  (SYS_MEM_RESERVE_STARTUP_STACK_SIZE + OS_MEM_NODE_HEAD_SIZE_TWO + \
                                  sizeof(struct LosHeapManager) + 1)
#define OS_MEM_NODE_HEAD_SIZE_TWO (2 * OS_MEM_NODE_HEAD_SIZE)
#endif
#define OS_MEM_MIN_POOL_SIZE  (OS_MEM_NODE_HEAD_SIZE + sizeof(struct LosHeapManager) + 1)

typedef struct tagLosHeapStatus {
    UINT32 totalUsedSize;
    UINT32 totalFreeSize;
    UINT32 maxFreeNodeSize;
    UINT32 usedNodeNum;
    UINT32 freeNodeNum;
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 usageWaterLine;
#endif
} LosHeapStatus;
#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
#define NODE_USED_STAT(node)              NodeUsed(node)
#define NODE_PREV(heapMgr, node)          NodePrev(heapMgr, node)
#define SET_NODE_PREV(heapMgr, node, ptr) SetNodePrev(heapMgr, node, ptr)
#define NODE_SIZE(node)                   NodeSize(node)
#define SET_NODE_SIZE(node, sz)           SetNodeSize(node, sz)
#define SET_NODE_USED_STAT(node, is_used) SetNodeUsed(node, is_used)
#define NODE_DATA(node)                   NodeData(node)
#define NODE_HEAD_SIZE(node)              NodeHeadSize(node)
#define NODE_TYPE(node)        ((node)->small.node_type)
#define NODE_ALIGN(node)       ((node)->small.align)
#define SET_NODE_ALIGN(node, is_align) ((node)->small.align = (UINT32)(is_align))
#define SET_NODE_TYPE(node, type)         ((node)->small.node_type = (UINT32)(type))
#ifdef LOSCFG_MEM_TASK_STAT
#define NODE_TASKID(node) \
    ((NODE_TYPE(node) == LOS_MEM_NODE_TYPE_SMALL) ? (node)->small.taskId : (node)->large.taskId)
#define SET_NODE_TASKID(node, tid) \
    do { \
        if (NODE_TYPE(node) == LOS_MEM_NODE_TYPE_SMALL) { \
            (node)->small.taskId = (UINT32)tid; \
        } else { \
            (node)->large.taskId = (UINT32)tid; \
        } \
    } while (0)
#endif
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
#define NODE_CALLER_RA(node)   NodeCallerRa(node)
#define SET_NODE_CALLER_RA(node, ra)   SetNodeCallerRa(node, ra)
#endif
#else
#define NODE_TYPE(node)        1U
#define NODE_PREV(heapMan, node) ((node)->prev)
#define SET_NODE_PREV(heapMan, node, ptr) ((node)->prev = (LosHeapNode*)(ptr))
#define NODE_SIZE(node)        ((node)->size)
#define SET_NODE_SIZE(node, sz) ((node)->size = (UINT32)(sz))
#define NODE_USED_STAT(node)        ((node)->used)
#define SET_NODE_USED_STAT(node, is_used) ((node)->used = (UINT32)(is_used))
#define NODE_ALIGN(node)       ((node)->align)
#define SET_NODE_ALIGN(node, is_align) ((node)->align = (UINT32)(is_align))
#define SET_NODE_TYPE(node, type)  ((void)(type))
#define NODE_DATA(node)        ((UINT8*)(node) + sizeof(LosHeapNode))
#define NODE_HEAD_SIZE(node)   (sizeof(LosHeapNode))
#ifdef LOSCFG_MEM_TASK_STAT
#define NODE_TASKID(node)      ((node)->taskId)
#define SET_NODE_TASKID(node, tid) ((node)->taskId = (UINT32)(tid))
#endif
#if defined(LOSCFG_MEM_DFX_SHOW_CALLER_RA) && defined(LOSCFG_MEM_TASK_STAT)
#define NODE_CALLER_RA(node)   ((node)->callerRa)
#define SET_NODE_CALLER_RA(node, ra)   ((node)->callerRa = (ra))
#endif
#endif /* LOSCFG_HEAP_NODE_SIZE_OPT */

extern UINTPTR g_callerRA[TASK_NUM];

extern BOOL OsHeapInit(VOID *pool, UINT32 size);
extern VOID* OsHeapAlloc(VOID *pool, UINT32 size);
extern VOID* OsHeapAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);
extern BOOL OsHeapFree(VOID *pool, const VOID* ptr);
extern UINT32 OsHeapStatisticsGet(VOID *pool, LosHeapStatus *status);
extern UINT32 OsHeapIntegrityCheck(struct LosHeapManager *heap);
extern UINTPTR OsHeapLastUsedGet(VOID *pool);
extern UINT32 OsHeapTaskHeapInfoGet(VOID *pool, UINT8 taskId, VOID (*print)(const char *fmt, ...));
extern VOID OsSaveCallerRa(UINTPTR callerRa);
extern BOOL OsIsSetCallerRa(VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_INTERNAL_H */
