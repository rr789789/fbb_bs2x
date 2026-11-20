/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2024. All rights reserved.
 * Description: LiteOS 通用对象分配/释放实现
 * --------------------------------------------------------------------------- */
#include "los_obj_alloc.h"
#include "los_memstat_pri.h"
#include "los_membox.h"
#include "los_task_base.h"
 
 
#if LOSCFG_KERNEL_OBJ_DYNAMIC_ALLOC
static OsObjPoolCtx g_objPoolCtx[OS_OBJ_TYPE_MAX] = {0};
 
UINT32 OsObjPoolInit(OsObjType type, UINT32 blkSize, UINT32 baseNum)
{
    if (type >= OS_OBJ_TYPE_MAX || blkSize == 0) {
        PRINT_ERR("invalid input parameter, %s\n", __func__);
        return LOS_NOK;
    }
 
    OsObjPoolCtx *ctx = &g_objPoolCtx[type];
    if (ctx->poolBuf != NULL) {
        PRINT_ERR("obj pool already initialized, %s\n", __func__);
        return LOS_NOK;
    }
    ctx->blkSize = blkSize;
    ctx->baseNum = baseNum;
    ctx->dynamicCnt = 0;
 
    ctx->poolSize = LOS_MEMBOX_SIZE(blkSize, baseNum);
    ctx->poolBuf = (UINT8 *)LOS_MemAlloc(m_aucSysMem0, ctx->poolSize);
    if (ctx->poolBuf == NULL) {
        PRINT_ERR("pool buf alloc fail\n");
        return LOS_NOK;
    }
 
    UINT32 ret = LOS_MemboxInit(ctx->poolBuf, ctx->poolSize, blkSize);
    if (ret != LOS_OK) {
        LOS_MemFree(m_aucSysMem0, ctx->poolBuf);
        ctx->poolBuf = NULL;
        PRINT_ERR("membox alloc fail\n");
        return ret;
    }
    return LOS_OK;
}
 
VOID *OsObjAlloc(OsObjType type)
{
    UINT32 intSave;
 
    if (type >= OS_OBJ_TYPE_MAX) {
        PRINT_ERR("invalid input parameter, %s\n", __func__);
        return NULL;
    }
 
    OsObjPoolCtx *ctx = &g_objPoolCtx[type];
    VOID *obj = NULL;
 
    if (ctx->poolBuf != NULL) {
        obj = LOS_MemboxAlloc(ctx->poolBuf);
        if (obj != NULL) {
            return obj;
        }
    }
 
    obj = LOS_MemAlloc(m_aucSysMem0, ctx->blkSize + OS_OBJ_DYNAMIC_META_SIZE);
    if (obj != NULL) {
        (VOID)memset_s((UINT8 *)obj, ctx->blkSize + OS_OBJ_DYNAMIC_META_SIZE,
            0, ctx->blkSize + OS_OBJ_DYNAMIC_META_SIZE);
        *(UINT8 *)obj = type;
        SCHEDULER_LOCK(intSave);
        ctx->dynamicCnt++;
        SCHEDULER_UNLOCK(intSave);
        obj = (UINT8 *)obj + OS_OBJ_DYNAMIC_META_SIZE;
    }
    return obj;
}
 
UINT32 OsObjFree(OsObjType type, VOID *obj)
{
    UINT32 intSave;
 
    if (type >= OS_OBJ_TYPE_MAX || obj == NULL) {
        PRINT_ERR("invalid input parameter, %s\n", __func__);
        return LOS_NOK;
    }
 
    OsObjPoolCtx *ctx = &g_objPoolCtx[type];
    UINT32 ret;
 
    if (ctx->poolBuf != NULL && (UINT8 *)obj >= ctx->poolBuf && (UINT8 *)obj < ctx->poolBuf + ctx->poolSize) {
        ret = LOS_MemboxFree(ctx->poolBuf, obj);
    } else {
        UINT8 *meta = (UINT8 *)obj - OS_OBJ_DYNAMIC_META_SIZE;
        ret = LOS_MemFree(m_aucSysMem0, meta);
        if (ret == LOS_OK) {
            SCHEDULER_LOCK(intSave);
            ctx->dynamicCnt--;
            SCHEDULER_UNLOCK(intSave);
        }
    }
    return ret;
}
 
VOID OsObjPoolStat(VOID)
{
    OsObjType type;
    UINT32 totalBlk, usedBlk, blkSz;
    UINT32 dynamicUsed;
    OsObjPoolCtx *ctx = NULL;
    for (type = OS_OBJ_TYPE_TASKCB; type < OS_OBJ_TYPE_MAX; type++) {
        ctx = &g_objPoolCtx[type];
        if (ctx->poolBuf != NULL) {
            LOS_MemboxStatisticsGet(ctx->poolBuf, &totalBlk, &usedBlk, &blkSz);
            PRINTK("pool type:%d, sz:%d, buf:%p, totalBlk:%d, used:%d, blksz:%d\n",
                type, ctx->poolSize, ctx->poolBuf, totalBlk, usedBlk, blkSz);
        }
        dynamicUsed = ctx->dynamicCnt;
        PRINTK("pool type:%d, dynamicUsed:%d\n", type, dynamicUsed);
    }
}
#endif /* LOSCFG_KERNEL_OBJ_DYNAMIC_ALLOC */