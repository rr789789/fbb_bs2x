/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2026. All rights reserved.
 */
/* ----------------------------------------------------------------------------
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2020-2024. All rights reserved.
 * Description: LiteOS 对象模板实现
 * Author: Huawei LiteOS Team
 * Create: 2020-01-01
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

#ifndef _LOS_OBJ_TEMPLATE_H
#define _LOS_OBJ_TEMPLATE_H

#include <stddef.h>
#include "securec.h"
#include "los_obj_alloc.h"
#include "los_bitmap.h"
#include "los_mux.h"
#include "los_typedef.h"
#include "los_printf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_BITMAP_MAX_BITS 32

#define DEFINE_OBJ_TEMPLATE_TYPE(obj_name, max_num) \
    typedef struct { \
        UINT32  id_bitmap; \
        void    *id_map[max_num]; \
        UINT32  mux; \
        UINT32  limit; \
        UINT32  inited; \
    } LosObjTemplate_##obj_name

#define OBJ_TEMPLATE_DECLARE(obj_name) \
    extern LosObjTemplate_##obj_name obj_template_##obj_name

#define OBJ_TEMPLATE_DEFINE(obj_type, obj_name, max_num, base_num, obj_size)                              \
    LosObjTemplate_##obj_name obj_template_##obj_name = {0};                                              \
    static UINT32 obj_template_##obj_name##_init(void)                                                    \
    {                                                                                                     \
        if (obj_template_##obj_name.inited)                                                               \
            return LOS_OK;                                                                                \
        obj_template_##obj_name.id_bitmap = 0;                                                            \
        obj_template_##obj_name.limit = max_num;                                                          \
        (void)memset_s(obj_template_##obj_name.id_map, sizeof(obj_template_##obj_name.id_map), 0,         \
                       sizeof(obj_template_##obj_name.id_map));                                           \
        UINT32 mux_ret = LOS_MuxCreate(&obj_template_##obj_name.mux);                                     \
        if (mux_ret != LOS_OK)                                                                            \
            return mux_ret;                                                                               \
        UINT32 ret = OsObjPoolInit(obj_type, obj_size, base_num);                                         \
        if (ret == LOS_OK) {                                                                              \
            obj_template_##obj_name.inited = 1;                                                           \
        }                                                                                                 \
        return ret;                                                                                       \
    }                                                                                                     \
    static void *obj_template_##obj_name##_alloc(UINT32 *obj_id)                                          \
    {                                                                                                     \
        if (obj_id == NULL) {                                                                               \
            return NULL;                                                                                  \
        }                                                                                                   \
        if (obj_template_##obj_name##_init() != LOS_OK) {                                                   \
            return NULL;                                                                                  \
        }                                                                                                   \
        LOS_MuxPend(obj_template_##obj_name.mux, LOS_WAIT_FOREVER);                                       \
        UINT32 id = LOS_FirstZeroGet(obj_template_##obj_name.id_bitmap);                                  \
        if (id >= OS_BITMAP_MAX_BITS) {                                                                   \
            LOS_MuxPost(obj_template_##obj_name.mux);                                                     \
            return NULL;                                                                                  \
        }                                                                                                 \
        LOS_BitmapSet(&obj_template_##obj_name.id_bitmap, id);                                            \
        LOS_MuxPost(obj_template_##obj_name.mux);                                                         \
        void *obj = OsObjAlloc(obj_type);                                                                 \
        if (obj != NULL) {                                                                                \
            LOS_MuxPend(obj_template_##obj_name.mux, LOS_WAIT_FOREVER);                                   \
            obj_template_##obj_name.id_map[id] = obj;                                                     \
            LOS_MuxPost(obj_template_##obj_name.mux);                                                     \
            *obj_id = id;                                                                                 \
            return obj;                                                                                   \
        }                                                                                                 \
        LOS_MuxPend(obj_template_##obj_name.mux, LOS_WAIT_FOREVER);                                       \
        LOS_BitmapClr(&obj_template_##obj_name.id_bitmap, id);                                            \
        LOS_MuxPost(obj_template_##obj_name.mux);                                                         \
        return NULL;                                                                                      \
    }                                                                                                     \
    static void obj_template_##obj_name##_free(void *obj, UINT32 obj_id)                                  \
    {                                                                                                     \
        if (obj == NULL || obj_id >= (max_num)) {                                                         \
            return;                                                                                       \
        }                                                                                                 \
        LOS_MuxPend(obj_template_##obj_name.mux, LOS_WAIT_FOREVER);                                       \
        (void)OsObjFree(obj_type, obj);                                                                   \
        LOS_BitmapClr(&obj_template_##obj_name.id_bitmap, obj_id);                                        \
        obj_template_##obj_name.id_map[obj_id] = NULL;                                                    \
        LOS_MuxPost(obj_template_##obj_name.mux);                                                         \
    }
#define OBJ_FOREACH(obj_name, obj, id)                                                                        \
    UINT32 id;                                                                                                \
    typeof(obj) (obj);                                                                                          \
    for ((id) = 0; (id) < obj_template_##obj_name.limit; (id)++) {                                                  \
        if (!LOS_BitmapSet(&obj_name##_template.id_bitmap, id) || (obj_name##_template.id_map[id] == NULL)) { \
            continue;                                                                                         \
        }                                                                                                     \
        (obj) = (typeof(obj))obj_name##_template.id_map[id];                                                    \
    }

#define OBJ_FROM_ID(obj_name, id, type) \
        (((UINT32)(id) < obj_template_##obj_name.limit) ? (type*)obj_template_##obj_name.id_map[id] : NULL)

#define DEFINE_OBJ_TEMPLATE(obj_type, obj_name, max_num, base_num, obj_size) \
    OBJ_TEMPLATE_DEFINE(obj_type, obj_name, max_num, base_num, obj_size)
#define ALLOC_OBJ(obj_name, obj_id) \
    obj_template_##obj_name##_alloc(obj_id)
#define FREE_OBJ(obj_name, obj, obj_id) \
    obj_template_##obj_name##_free(obj, obj_id)

#ifdef __cplusplus
}
#endif

#endif /* _LOS_OBJ_TEMPLATE_H */