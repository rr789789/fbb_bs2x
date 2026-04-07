/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: SLE-BLE Gateway Service Layer -- connection table, route table, message queue. \n
 */
#include "common_def.h"
#include "securec.h"
#include "soc_osal.h"
#include "gateway_service.h"
#include "gw_sle_adapter.h"
#include "gw_ble_adapter.h"

#define GW_LOG                  "[gateway] "
#define GW_MSG_QUEUE_LEN        16
#define GW_QUEUE_WAIT_FOREVER   0xFFFFFFFF

/* -------------------------------------------------------------------------
 * Module state
 * ---------------------------------------------------------------------- */
static unsigned long  g_gw_msg_queue_id                    = 0;
static gw_conn_ctx_t  g_gw_conn_table[GW_MAX_CONNECTIONS];
static uint8_t        g_gw_route_table[GW_MAX_CONNECTIONS]; /* [src] -> dst slot */
static gw_event_cb_t  g_on_connect_cb                      = NULL;
static gw_event_cb_t  g_on_disconnect_cb                   = NULL;

/* -------------------------------------------------------------------------
 * Service lifecycle
 * ---------------------------------------------------------------------- */
errcode_t gw_service_init(gw_event_cb_t on_connect_cb, gw_event_cb_t on_disconnect_cb)
{
    g_on_connect_cb    = on_connect_cb;
    g_on_disconnect_cb = on_disconnect_cb;

    for (uint8_t i = 0; i < GW_MAX_CONNECTIONS; i++) {
        g_gw_conn_table[i].slot  = i;
        g_gw_conn_table[i].state = GW_CONN_STATE_IDLE;
        g_gw_route_table[i]      = GW_MAX_CONNECTIONS; /* invalid / no route */
    }

    int ret = osal_msg_queue_create("gw_queue", GW_MSG_QUEUE_LEN,
                                    (unsigned long *)&g_gw_msg_queue_id,
                                    0, (unsigned short)sizeof(gw_msg_t));
    if (ret != OSAL_SUCCESS) {
        osal_printk("%s create msg queue fail\r\n", GW_LOG);
        return ERRCODE_FAIL;
    }
    osal_printk("%s service init ok\r\n", GW_LOG);
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * Connection table
 * ---------------------------------------------------------------------- */
uint8_t gw_conn_alloc(gw_protocol_t protocol, uint16_t proto_conn_id,
                      uint16_t attr_handle, const uint8_t *peer_addr)
{
    for (uint8_t i = 0; i < GW_MAX_CONNECTIONS; i++) {
        if (g_gw_conn_table[i].state == GW_CONN_STATE_IDLE) {
            g_gw_conn_table[i].protocol      = protocol;
            g_gw_conn_table[i].proto_conn_id = proto_conn_id;
            g_gw_conn_table[i].attr_handle   = attr_handle;
            g_gw_conn_table[i].state         = GW_CONN_STATE_CONNECTED;
            if (peer_addr != NULL) {
                if (memcpy_s(g_gw_conn_table[i].peer_addr, sizeof(g_gw_conn_table[i].peer_addr),
                             peer_addr, 6) != EOK) {
                    (void)memset_s(g_gw_conn_table[i].peer_addr,
                                   sizeof(g_gw_conn_table[i].peer_addr), 0, 6);
                }
            }
            return i;
        }
    }
    return GW_MAX_CONNECTIONS; /* table full */
}

void gw_conn_free(uint8_t slot)
{
    if (slot >= GW_MAX_CONNECTIONS) {
        return;
    }
    (void)memset_s(&g_gw_conn_table[slot], sizeof(gw_conn_ctx_t), 0, sizeof(gw_conn_ctx_t));
    g_gw_conn_table[slot].slot  = slot;
    g_gw_conn_table[slot].state = GW_CONN_STATE_IDLE;
    g_gw_route_table[slot]      = GW_MAX_CONNECTIONS;
}

uint8_t gw_conn_find_by_id(gw_protocol_t protocol, uint16_t proto_conn_id)
{
    for (uint8_t i = 0; i < GW_MAX_CONNECTIONS; i++) {
        if (g_gw_conn_table[i].state         != GW_CONN_STATE_IDLE &&
            g_gw_conn_table[i].protocol      == protocol            &&
            g_gw_conn_table[i].proto_conn_id == proto_conn_id) {
            return i;
        }
    }
    return GW_MAX_CONNECTIONS;
}

const gw_conn_ctx_t *gw_conn_get(uint8_t slot)
{
    if (slot >= GW_MAX_CONNECTIONS) {
        return NULL;
    }
    return &g_gw_conn_table[slot];
}

/* -------------------------------------------------------------------------
 * Route table
 * ---------------------------------------------------------------------- */
errcode_t gw_route_add(uint8_t src_slot, uint8_t dst_slot)
{
    if (src_slot >= GW_MAX_CONNECTIONS || dst_slot >= GW_MAX_CONNECTIONS) {
        return ERRCODE_FAIL;
    }
    g_gw_route_table[src_slot] = dst_slot;
    return ERRCODE_SUCC;
}

void gw_route_remove(uint8_t src_slot)
{
    if (src_slot < GW_MAX_CONNECTIONS) {
        g_gw_route_table[src_slot] = GW_MAX_CONNECTIONS;
    }
}

uint8_t gw_route_lookup(uint8_t src_slot)
{
    if (src_slot >= GW_MAX_CONNECTIONS) {
        return GW_MAX_CONNECTIONS;
    }
    return g_gw_route_table[src_slot];
}

/* -------------------------------------------------------------------------
 * Message queue post (called from adapter callbacks, may be in ISR context)
 * ---------------------------------------------------------------------- */
errcode_t gw_service_post_msg(const gw_msg_t *msg)
{
    if (msg == NULL) {
        return ERRCODE_FAIL;
    }
    int ret = osal_msg_queue_write_copy(g_gw_msg_queue_id,
                                        (void *)msg,
                                        (uint32_t)sizeof(gw_msg_t),
                                        0);
    if (ret != OSAL_SUCCESS) {
        osal_printk("%s post msg fail, type=%d\r\n", GW_LOG, (int)msg->type);
        return ERRCODE_FAIL;
    }
    return ERRCODE_SUCC;
}

/* -------------------------------------------------------------------------
 * Internal event handlers
 * ---------------------------------------------------------------------- */
static void gw_handle_connected(const gw_msg_t *msg)
{
    uint8_t slot = gw_conn_alloc(msg->protocol, msg->proto_conn_id,
                                 msg->attr_handle, msg->peer_addr);
    if (slot >= GW_MAX_CONNECTIONS) {
        osal_printk("%s conn table full, proto=%d conn_id=%d\r\n",
                    GW_LOG, (int)msg->protocol, msg->proto_conn_id);
        return;
    }
    osal_printk("%s connected: proto=%d conn_id=%d slot=%d\r\n",
                GW_LOG, (int)msg->protocol, msg->proto_conn_id, slot);
    if (g_on_connect_cb != NULL) {
        g_on_connect_cb(&g_gw_conn_table[slot]);
    }
}

static void gw_handle_disconnected(const gw_msg_t *msg)
{
    uint8_t slot = gw_conn_find_by_id(msg->protocol, msg->proto_conn_id);
    if (slot >= GW_MAX_CONNECTIONS) {
        return;
    }
    osal_printk("%s disconnected: proto=%d conn_id=%d slot=%d\r\n",
                GW_LOG, (int)msg->protocol, msg->proto_conn_id, slot);
    /* snapshot before free */
    gw_conn_ctx_t ctx = g_gw_conn_table[slot];
    gw_conn_free(slot);
    if (g_on_disconnect_cb != NULL) {
        g_on_disconnect_cb(&ctx);
    }
}

static void gw_handle_data(const gw_msg_t *msg)
{
    uint8_t src_slot = gw_conn_find_by_id(msg->protocol, msg->proto_conn_id);
    if (src_slot >= GW_MAX_CONNECTIONS) {
        osal_printk("%s data from unknown conn proto=%d conn_id=%d\r\n",
                    GW_LOG, (int)msg->protocol, msg->proto_conn_id);
        return;
    }
    uint8_t dst_slot = gw_route_lookup(src_slot);
    if (dst_slot >= GW_MAX_CONNECTIONS) {
        osal_printk("%s no route from slot %d\r\n", GW_LOG, src_slot);
        return;
    }
    const gw_conn_ctx_t *dst = gw_conn_get(dst_slot);
    if (dst == NULL || dst->state != GW_CONN_STATE_CONNECTED) {
        osal_printk("%s dst slot %d not connected\r\n", GW_LOG, dst_slot);
        return;
    }
    if (dst->protocol == GW_PROTO_SLE) {
        gw_sle_adapter_send(dst->proto_conn_id, dst->attr_handle,
                            msg->data, msg->data_len);
    } else if (dst->protocol == GW_PROTO_BLE) {
        gw_ble_adapter_send(dst->proto_conn_id, dst->attr_handle,
                            msg->data, msg->data_len);
    }
}

/* -------------------------------------------------------------------------
 * Event loop (called in a tight loop from the gateway task)
 * ---------------------------------------------------------------------- */
errcode_t gw_service_process_events(void)
{
    gw_msg_t msg     = {0};
    uint32_t msg_size = (uint32_t)sizeof(msg);

    int ret = osal_msg_queue_read_copy(g_gw_msg_queue_id, &msg,
                                       &msg_size, GW_QUEUE_WAIT_FOREVER);
    if (ret != OSAL_SUCCESS) {
        return ERRCODE_FAIL;
    }

    switch (msg.type) {
        case GW_MSG_CONN_CONNECTED:
            gw_handle_connected(&msg);
            break;
        case GW_MSG_CONN_DISCONNECTED:
            gw_handle_disconnected(&msg);
            break;
        case GW_MSG_DATA_RECEIVED:
            gw_handle_data(&msg);
            break;
        default:
            osal_printk("%s unknown msg type 0x%x\r\n", GW_LOG, (unsigned)msg.type);
            break;
    }
    return ERRCODE_SUCC;
}
