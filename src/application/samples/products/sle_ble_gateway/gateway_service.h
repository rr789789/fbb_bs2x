/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE-BLE Gateway Service Layer -- public API. \n
 */
#ifndef GATEWAY_SERVICE_H
#define GATEWAY_SERVICE_H

#include <stdint.h>
#include "errcode.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* -------------------------------------------------------------------------
 * Compile-time constants (overridden by Kconfig)
 * ---------------------------------------------------------------------- */
#ifndef CONFIG_GW_MAX_CONNECTIONS
#define GW_MAX_CONNECTIONS  8
#else
#define GW_MAX_CONNECTIONS  CONFIG_GW_MAX_CONNECTIONS
#endif

#ifndef CONFIG_GW_DATA_MAX_LEN
#define GW_DATA_MAX_LEN     256
#else
#define GW_DATA_MAX_LEN     CONFIG_GW_DATA_MAX_LEN
#endif

/* -------------------------------------------------------------------------
 * Protocol and state enumerations
 * ---------------------------------------------------------------------- */
typedef enum {
    GW_PROTO_NONE = 0x00,
    GW_PROTO_SLE  = 0x01,
    GW_PROTO_BLE  = 0x02,
} gw_protocol_t;

typedef enum {
    GW_CONN_STATE_IDLE      = 0x00,
    GW_CONN_STATE_CONNECTED = 0x01,
} gw_conn_state_t;

/* -------------------------------------------------------------------------
 * Connection context (one per active connection)
 * ---------------------------------------------------------------------- */
typedef struct {
    uint8_t          slot;           /* index in connection table */
    gw_protocol_t    protocol;
    gw_conn_state_t  state;
    uint16_t         proto_conn_id;  /* stack-level connection ID */
    uint16_t         attr_handle;    /* SSAP property / GATT characteristic value handle */
    uint8_t          peer_addr[6];
} gw_conn_ctx_t;

/* -------------------------------------------------------------------------
 * Message types and structure (serialised through the OSAL queue)
 * ---------------------------------------------------------------------- */
typedef enum {
    GW_MSG_CONN_CONNECTED    = 0x01,
    GW_MSG_CONN_DISCONNECTED = 0x02,
    GW_MSG_DATA_RECEIVED     = 0x10,
} gw_msg_type_t;

typedef struct {
    gw_msg_type_t  type;
    gw_protocol_t  protocol;
    uint16_t       proto_conn_id;
    uint16_t       attr_handle;   /* used in CONNECTED events */
    uint16_t       data_len;      /* used in DATA_RECEIVED events */
    uint8_t        peer_addr[6];  /* used in CONNECTED events */
    uint8_t        data[GW_DATA_MAX_LEN];
} gw_msg_t;

/* -------------------------------------------------------------------------
 * Event callbacks (registered by the demo layer)
 * ---------------------------------------------------------------------- */
typedef void (*gw_event_cb_t)(const gw_conn_ctx_t *conn);

/* -------------------------------------------------------------------------
 * Service lifecycle
 * ---------------------------------------------------------------------- */
errcode_t gw_service_init(gw_event_cb_t on_connect_cb, gw_event_cb_t on_disconnect_cb);

/**
 * @brief Block on the internal message queue and process one event.
 *        Call in a tight loop from the gateway task.
 */
errcode_t gw_service_process_events(void);

/* -------------------------------------------------------------------------
 * Connection table management (called by protocol adapters)
 * ---------------------------------------------------------------------- */
uint8_t   gw_conn_alloc(gw_protocol_t protocol, uint16_t proto_conn_id,
                        uint16_t attr_handle, const uint8_t *peer_addr);
void      gw_conn_free(uint8_t slot);
uint8_t   gw_conn_find_by_id(gw_protocol_t protocol, uint16_t proto_conn_id);
const gw_conn_ctx_t *gw_conn_get(uint8_t slot);

/* -------------------------------------------------------------------------
 * Route management
 * ---------------------------------------------------------------------- */
errcode_t gw_route_add(uint8_t src_slot, uint8_t dst_slot);
void      gw_route_remove(uint8_t src_slot);
uint8_t   gw_route_lookup(uint8_t src_slot);

/* -------------------------------------------------------------------------
 * Post a message from adapter callbacks (non-blocking, ISR/callback safe)
 * ---------------------------------------------------------------------- */
errcode_t gw_service_post_msg(const gw_msg_t *msg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* GATEWAY_SERVICE_H */
