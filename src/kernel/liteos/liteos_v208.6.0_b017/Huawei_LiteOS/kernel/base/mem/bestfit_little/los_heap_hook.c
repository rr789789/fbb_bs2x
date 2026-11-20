/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2021. All rights reserved.
 */
#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
#include "los_memory_pri.h"
#include "los_memory_internal.h"

union LosHeapNode *NodePrev(const struct LosHeapManager *heapMgr, const union LosHeapNode *node)
{
    if (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) {
        UINT32 offset = node->small.prev_offset;
        return (offset == 0) ? NULL : (union LosHeapNode *)(heapMgr->heapBaseAddr + offset);
    }
    return (union LosHeapNode *)node->large.prev;
}

void SetNodePrev(struct LosHeapManager *heapMgr, union LosHeapNode *node, union LosHeapNode *ptr)
{
    if (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) {
        node->small.prev_offset = (ptr == NULL) ? 0 : (UINT32)((UINTPTR)ptr - heapMgr->heapBaseAddr);
    } else {
        node->large.prev = (struct LosHeapNodeLarge *)ptr;
    }
}

UINT32 NodeSize(const union LosHeapNode *node)
{
    return (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) ? node->small.size : node->large.size;
}

void SetNodeSize(union LosHeapNode *node, UINT32 sz)
{
    if (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) {
        node->small.size = sz & LOS_MEM_SMALL_NODE_SIZE_MAX;
    } else {
        node->large.size = sz;
    }
}

UINT8 NodeUsed(const union LosHeapNode *node)
{
    return (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) ? node->small.used : node->large.used;
}

void SetNodeUsed(union LosHeapNode *node, UINT32 is_used)
{
    if (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) {
        node->small.used = is_used;
    } else {
        node->large.used = is_used;
    }
}

UINT8 *NodeData(const union LosHeapNode *node)
{
    return (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) ?
           (UINT8 *)node + sizeof(struct LosHeapNodeSmall) :
           (UINT8 *)node + sizeof(struct LosHeapNodeLarge);
}

UINT32 NodeHeadSize(const union LosHeapNode *node)
{
    return (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) ?
           sizeof(struct LosHeapNodeSmall) : sizeof(struct LosHeapNodeLarge);
}

BOOL OsHeapNodeValid(const struct LosHeapManager *heapMan, const union LosHeapNode *node)
{
    if (node == NULL) {
        return FALSE;
    }
    if ((node != heapMan->tail) && (node != heapMan->head)) {
        const union LosHeapNode *prev = NodePrev(heapMan, node);
        if (prev == NULL) {
            return FALSE;
        }
        if ((UINTPTR)OsHeapPrvGetNext(heapMan, prev) != (UINTPTR)node) {
            return FALSE;
        }
    }
    return TRUE;
}
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
UINTPTR NodeCallerRa(const union LosHeapNode *node)
{
    return (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) ? node->small.callerRa : node->large.callerRa;
}

void SetNodeCallerRa(union LosHeapNode *node, UINTPTR callerRa)
{
    if (node->small.node_type == LOS_MEM_NODE_TYPE_SMALL) {
        node->small.callerRa = callerRa;
    } else {
        node->large.callerRa = callerRa;
    }
}
#endif
#endif