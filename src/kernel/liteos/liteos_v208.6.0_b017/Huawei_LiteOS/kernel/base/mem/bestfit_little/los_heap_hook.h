/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2021. All rights reserved.
 */
#ifndef _LOS_HEAP_HOOK_H
#define _LOS_HEAP_HOOK_H

#ifdef LOSCFG_HEAP_NODE_SIZE_OPT
#include "los_typedef.h"
#include "los_memory_pri.h"

union LosHeapNode *NodePrev(const struct LosHeapManager *heapMgr, const union LosHeapNode *node);
void SetNodePrev(struct LosHeapManager *heapMgr, union LosHeapNode *node, union LosHeapNode *ptr);
UINT32 NodeSize(const union LosHeapNode *node);
void SetNodeSize(union LosHeapNode *node, UINT32 sz);
UINT8 NodeUsed(const union LosHeapNode *node);
void SetNodeUsed(union LosHeapNode *node, UINT32 is_used);
UINT8 *NodeData(const union LosHeapNode *node);
UINT32 NodeHeadSize(const union LosHeapNode *node);
BOOL OsHeapNodeValid(const struct LosHeapManager *heapMan, const union LosHeapNode *node);
union LosHeapNode *OsHeapPrvGetNext(const struct LosHeapManager *heapMan, const union LosHeapNode *node);
#ifdef LOSCFG_MEM_DFX_SHOW_CALLER_RA
UINTPTR NodeCallerRa(const union LosHeapNode *node);
void SetNodeCallerRa(union LosHeapNode *node, UINTPTR callerRa);
#endif

#endif
#endif