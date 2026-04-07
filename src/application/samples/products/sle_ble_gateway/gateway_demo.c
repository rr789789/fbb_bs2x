/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Gateway demo -- automatic SLE <-> BLE routing. \n
 *
 * Strategy:
 *   - Track the first connected SLE slot and the first connected BLE slot.
 *   - As soon as both are available, establish a bidirectional route between them.
 *   - When either disconnects, remove both routes so the next connection can be
 *     routed to whatever remains.
 */
#include "soc_osal.h"
#include "gateway_service.h"
#include "gateway_demo.h"

#define GW_DEMO_LOG  "[gw_demo] "

static uint8_t g_sle_slot = GW_MAX_CONNECTIONS; /* invalid sentinel */
static uint8_t g_ble_slot = GW_MAX_CONNECTIONS;

static void try_setup_route(void)
{
    if (g_sle_slot >= GW_MAX_CONNECTIONS || g_ble_slot >= GW_MAX_CONNECTIONS) {
        return; /* one side not yet connected */
    }
    gw_route_add(g_sle_slot, g_ble_slot);
    gw_route_add(g_ble_slot, g_sle_slot);
    osal_printk("%s route: SLE[slot=%d] <-> BLE[slot=%d]\r\n",
                GW_DEMO_LOG, g_sle_slot, g_ble_slot);
}

void gateway_demo_on_connect(const gw_conn_ctx_t *conn)
{
    if (conn == NULL) {
        return;
    }
    if (conn->protocol == GW_PROTO_SLE) {
        if (g_sle_slot >= GW_MAX_CONNECTIONS) {
            g_sle_slot = conn->slot;
            osal_printk("%s SLE connected, slot=%d\r\n", GW_DEMO_LOG, g_sle_slot);
        } else {
            osal_printk("%s extra SLE connection slot=%d, no route assigned\r\n",
                        GW_DEMO_LOG, conn->slot);
            return;
        }
    } else if (conn->protocol == GW_PROTO_BLE) {
        if (g_ble_slot >= GW_MAX_CONNECTIONS) {
            g_ble_slot = conn->slot;
            osal_printk("%s BLE connected, slot=%d\r\n", GW_DEMO_LOG, g_ble_slot);
        } else {
            osal_printk("%s extra BLE connection slot=%d, no route assigned\r\n",
                        GW_DEMO_LOG, conn->slot);
            return;
        }
    }
    try_setup_route();
}

void gateway_demo_on_disconnect(const gw_conn_ctx_t *conn)
{
    if (conn == NULL) {
        return;
    }
    osal_printk("%s disconnect: proto=%d slot=%d\r\n",
                GW_DEMO_LOG, (int)conn->protocol, conn->slot);

    if (conn->slot == g_sle_slot) {
        gw_route_remove(g_sle_slot);
        if (g_ble_slot < GW_MAX_CONNECTIONS) {
            gw_route_remove(g_ble_slot);
        }
        g_sle_slot = GW_MAX_CONNECTIONS;
    } else if (conn->slot == g_ble_slot) {
        gw_route_remove(g_ble_slot);
        if (g_sle_slot < GW_MAX_CONNECTIONS) {
            gw_route_remove(g_sle_slot);
        }
        g_ble_slot = GW_MAX_CONNECTIONS;
    }
}
