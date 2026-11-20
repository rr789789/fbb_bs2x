/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2024. All rights reserved.
 * Description: LiteOS 通用对象分配/释放接口
 * --------------------------------------------------------------------------- */
#ifndef _LOS_OBJ_ALLOC_H
#define _LOS_OBJ_ALLOC_H
 
#include "los_config.h"
#include "los_membox.h"
#include "los_memory.h"
#include "los_task_base.h"
 
#ifdef __cplusplus
extern "C" {
#endif
/* 内核对象类型枚举 */
typedef enum {
    OS_OBJ_TYPE_TASKCB,
    OS_OBJ_TYPE_SWTMR,
    OS_OBJ_TYPE_QUEUECB,
    OS_OBJ_TYPE_SEMCB,
    OS_OBJ_TYPE_MAX
} OsObjType;
 
typedef struct {
    UINT8 *poolBuf;            // membox缓冲区
    UINT32 blkSize;            // 单个对象大小
    UINT32 baseNum;            // 基础预分配数量
    UINT32 poolSize;            // 内存池总大小
    UINT32 dynamicCnt;         // 动态分配计数
} OsObjPoolCtx;
 
#if LOSCFG_KERNEL_OBJ_DYNAMIC_ALLOC
#ifndef LOS_MEMBOX_SIZE
#define LOS_MEMBOX_SIZE(blkSize, blkNum) \
    (sizeof(UINT32) + (((blkSize) + sizeof(UINTPTR) - 1) & (~(sizeof(UINTPTR) - 1))) * (blkNum))
#endif
extern UINT32 OsObjPoolInit(OsObjType type, UINT32 blkSize, UINT32 baseNum);
extern VOID *OsObjAlloc(OsObjType type);
extern UINT32 OsObjFree(OsObjType type, VOID *obj);
extern VOID OsObjPoolStat(VOID);
#else
#define OsObjPoolInit(type, blkSize, baseNum) LOS_OK
#define OsObjAlloc(type) NULL
#define OsObjFree(type, obj) LOS_NOK
#define OsObjPoolStat()
#endif
 
#ifdef __cplusplus
}
#endif
 
#endif /* _LOS_OBJ_ALLOC_H */