/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Unit tests for gateway_service.c (PC-side, no hardware). \n
 */
#include <string.h>
#include <stdio.h>
#include "gateway_service.h"
#include "test_utils.h"
#include "test_helpers.h"

/* -------------------------------------------------------------------------
 * Callback tracking
 * ---------------------------------------------------------------------- */
static int g_connect_calls;
static int g_disconnect_calls;
static gw_conn_ctx_t g_last_connect_ctx;
static gw_conn_ctx_t g_last_disconnect_ctx;

static void on_connect(const gw_conn_ctx_t *c)
{
    g_connect_calls++;
    g_last_connect_ctx = *c;
}
static void on_disconnect(const gw_conn_ctx_t *c)
{
    g_disconnect_calls++;
    g_last_disconnect_ctx = *c;
}

static void reset_callbacks(void)
{
    g_connect_calls    = 0;
    g_disconnect_calls = 0;
    memset(&g_last_connect_ctx,    0, sizeof(g_last_connect_ctx));
    memset(&g_last_disconnect_ctx, 0, sizeof(g_last_disconnect_ctx));
}

/* =========================================================================
 * Test: connection table -- basic alloc / find / free
 * ====================================================================== */
static void test_conn_alloc_find_free(void)
{
    gw_service_init(NULL, NULL);

    uint8_t slot = gw_conn_alloc(GW_PROTO_SLE, 0x0010, 0xAA01, NULL);
    TEST_ASSERT(slot < GW_MAX_CONNECTIONS);
    TEST_ASSERT(gw_conn_get(slot)->protocol      == GW_PROTO_SLE);
    TEST_ASSERT(gw_conn_get(slot)->proto_conn_id == 0x0010);
    TEST_ASSERT(gw_conn_get(slot)->attr_handle   == 0xAA01);
    TEST_ASSERT(gw_conn_get(slot)->state         == GW_CONN_STATE_CONNECTED);

    uint8_t found = gw_conn_find_by_id(GW_PROTO_SLE, 0x0010);
    TEST_ASSERT(found == slot);

    /* Different protocol, same conn_id should NOT match */
    uint8_t not_found = gw_conn_find_by_id(GW_PROTO_BLE, 0x0010);
    TEST_ASSERT(not_found == GW_MAX_CONNECTIONS);

    gw_conn_free(slot);
    TEST_ASSERT(gw_conn_get(slot)->state == GW_CONN_STATE_IDLE);
    TEST_ASSERT(gw_conn_find_by_id(GW_PROTO_SLE, 0x0010) == GW_MAX_CONNECTIONS);
}

/* =========================================================================
 * Test: connection table -- overflow
 * ====================================================================== */
static void test_conn_table_full(void)
{
    gw_service_init(NULL, NULL);

    uint8_t slots[GW_MAX_CONNECTIONS];
    for (int i = 0; i < GW_MAX_CONNECTIONS; i++) {
        slots[i] = gw_conn_alloc(GW_PROTO_SLE, (uint16_t)(0x100 + i), 0, NULL);
        TEST_ASSERT(slots[i] < GW_MAX_CONNECTIONS);
    }
    /* Next allocation should fail */
    uint8_t fail = gw_conn_alloc(GW_PROTO_SLE, 0x999, 0, NULL);
    TEST_ASSERT(fail == GW_MAX_CONNECTIONS);

    /* Free one and try again */
    gw_conn_free(slots[0]);
    uint8_t ok = gw_conn_alloc(GW_PROTO_SLE, 0x999, 0, NULL);
    TEST_ASSERT(ok < GW_MAX_CONNECTIONS);
}

/* =========================================================================
 * Test: route add / lookup / remove
 * ====================================================================== */
static void test_route_basic(void)
{
    gw_service_init(NULL, NULL);

    gw_conn_alloc(GW_PROTO_SLE, 0x01, 0xA1, NULL);
    gw_conn_alloc(GW_PROTO_BLE, 0x02, 0xB1, NULL);
    uint8_t s = gw_conn_find_by_id(GW_PROTO_SLE, 0x01);
    uint8_t b = gw_conn_find_by_id(GW_PROTO_BLE, 0x02);

    TEST_ASSERT(gw_route_add(s, b) == ERRCODE_SUCC);
    TEST_ASSERT(gw_route_lookup(s) == b);
    TEST_ASSERT(gw_route_lookup(b) == GW_MAX_CONNECTIONS); /* no reverse route yet */

    TEST_ASSERT(gw_route_add(b, s) == ERRCODE_SUCC);
    TEST_ASSERT(gw_route_lookup(b) == s);

    gw_route_remove(s);
    TEST_ASSERT(gw_route_lookup(s) == GW_MAX_CONNECTIONS);
    TEST_ASSERT(gw_route_lookup(b) == s); /* reverse still present */

    /* Out of range */
    TEST_ASSERT(gw_route_add(GW_MAX_CONNECTIONS, 0) != ERRCODE_SUCC);
}

/* =========================================================================
 * Test: process_events -- CONNECTED event populates connection table
 * ====================================================================== */
static void test_event_connected(void)
{
    reset_callbacks();
    gw_service_init(on_connect, on_disconnect);

    gw_msg_t msg = {0};
    msg.type          = GW_MSG_CONN_CONNECTED;
    msg.protocol      = GW_PROTO_SLE;
    msg.proto_conn_id = 0x0042;
    msg.attr_handle   = 0x1234;
    test_queue_inject(&msg);
    gw_service_process_events();

    TEST_ASSERT(g_connect_calls == 1);
    TEST_ASSERT(g_last_connect_ctx.protocol      == GW_PROTO_SLE);
    TEST_ASSERT(g_last_connect_ctx.proto_conn_id == 0x0042);
    TEST_ASSERT(gw_conn_find_by_id(GW_PROTO_SLE, 0x0042) < GW_MAX_CONNECTIONS);
}

/* =========================================================================
 * Test: process_events -- DISCONNECTED event frees slot and invokes callback
 * ====================================================================== */
static void test_event_disconnected(void)
{
    reset_callbacks();
    gw_service_init(on_connect, on_disconnect);

    /* First connect */
    gw_msg_t msg = {0};
    msg.type = GW_MSG_CONN_CONNECTED;
    msg.protocol = GW_PROTO_BLE;
    msg.proto_conn_id = 0x0007;
    test_queue_inject(&msg);
    gw_service_process_events();
    TEST_ASSERT(gw_conn_find_by_id(GW_PROTO_BLE, 0x0007) < GW_MAX_CONNECTIONS);

    /* Then disconnect */
    msg.type = GW_MSG_CONN_DISCONNECTED;
    test_queue_inject(&msg);
    gw_service_process_events();

    TEST_ASSERT(g_disconnect_calls == 1);
    TEST_ASSERT(g_last_disconnect_ctx.protocol      == GW_PROTO_BLE);
    TEST_ASSERT(g_last_disconnect_ctx.proto_conn_id == 0x0007);
    TEST_ASSERT(gw_conn_find_by_id(GW_PROTO_BLE, 0x0007) == GW_MAX_CONNECTIONS);
}

/* =========================================================================
 * Test: process_events -- DATA_RECEIVED is routed to destination
 * ====================================================================== */
static void test_event_data_sle_to_ble(void)
{
    gw_service_init(NULL, NULL);
    test_reset_sends();

    /* Set up two connections */
    uint8_t s = gw_conn_alloc(GW_PROTO_SLE, 0x0100, 0xA001, NULL);
    uint8_t b = gw_conn_alloc(GW_PROTO_BLE, 0x0200, 0xB001, NULL);
    TEST_ASSERT(s < GW_MAX_CONNECTIONS);
    TEST_ASSERT(b < GW_MAX_CONNECTIONS);
    gw_route_add(s, b);
    gw_route_add(b, s);

    /* SLE -> BLE */
    uint8_t payload[] = {0xDE, 0xAD, 0xBE, 0xEF};
    gw_msg_t msg = {0};
    msg.type          = GW_MSG_DATA_RECEIVED;
    msg.protocol      = GW_PROTO_SLE;
    msg.proto_conn_id = 0x0100;
    msg.data_len      = sizeof(payload);
    memcpy(msg.data, payload, sizeof(payload));
    test_queue_inject(&msg);
    gw_service_process_events();

    const mock_send_t *ble_send = test_get_last_ble_send();
    TEST_ASSERT(ble_send->called > 0);
    TEST_ASSERT(ble_send->conn_id == 0x0200);
    TEST_ASSERT(ble_send->data_len == sizeof(payload));
    TEST_ASSERT(memcmp(ble_send->data, payload, sizeof(payload)) == 0);

    /* SLE send should NOT have been called */
    const mock_send_t *sle_send = test_get_last_sle_send();
    TEST_ASSERT(sle_send->called == 0);
}

static void test_event_data_ble_to_sle(void)
{
    gw_service_init(NULL, NULL);
    test_reset_sends();

    uint8_t s = gw_conn_alloc(GW_PROTO_SLE, 0x0300, 0xA002, NULL);
    uint8_t b = gw_conn_alloc(GW_PROTO_BLE, 0x0400, 0xB002, NULL);
    gw_route_add(s, b);
    gw_route_add(b, s);

    uint8_t payload[] = {'H', 'e', 'l', 'l', 'o'};
    gw_msg_t msg = {0};
    msg.type          = GW_MSG_DATA_RECEIVED;
    msg.protocol      = GW_PROTO_BLE;
    msg.proto_conn_id = 0x0400;
    msg.data_len      = sizeof(payload);
    memcpy(msg.data, payload, sizeof(payload));
    test_queue_inject(&msg);
    gw_service_process_events();

    const mock_send_t *sle_send = test_get_last_sle_send();
    TEST_ASSERT(sle_send->called > 0);
    TEST_ASSERT(sle_send->conn_id == 0x0300);
    TEST_ASSERT(sle_send->data_len == sizeof(payload));
    TEST_ASSERT(memcmp(sle_send->data, payload, sizeof(payload)) == 0);
}

/* =========================================================================
 * Test: data to unknown connection is silently dropped
 * ====================================================================== */
static void test_data_unknown_conn(void)
{
    gw_service_init(NULL, NULL);
    test_reset_sends();

    gw_msg_t msg = {0};
    msg.type          = GW_MSG_DATA_RECEIVED;
    msg.protocol      = GW_PROTO_SLE;
    msg.proto_conn_id = 0xFFFF; /* no such connection */
    msg.data_len      = 4;
    test_queue_inject(&msg);
    gw_service_process_events();

    TEST_ASSERT(test_get_last_ble_send()->called == 0);
    TEST_ASSERT(test_get_last_sle_send()->called == 0);
}

/* =========================================================================
 * Test: data with no route configured is silently dropped
 * ====================================================================== */
static void test_data_no_route(void)
{
    gw_service_init(NULL, NULL);
    test_reset_sends();

    gw_conn_alloc(GW_PROTO_SLE, 0x0500, 0xA003, NULL);
    /* Deliberately NO gw_route_add() */

    gw_msg_t msg = {0};
    msg.type          = GW_MSG_DATA_RECEIVED;
    msg.protocol      = GW_PROTO_SLE;
    msg.proto_conn_id = 0x0500;
    msg.data_len      = 2;
    test_queue_inject(&msg);
    gw_service_process_events();

    TEST_ASSERT(test_get_last_ble_send()->called == 0);
    TEST_ASSERT(test_get_last_sle_send()->called == 0);
}

/* =========================================================================
 * Test: multiple connections routed independently
 * ====================================================================== */
static void test_multi_conn_routing(void)
{
    gw_service_init(NULL, NULL);
    test_reset_sends();

    uint8_t s1 = gw_conn_alloc(GW_PROTO_SLE, 0x01, 0xA1, NULL);
    uint8_t s2 = gw_conn_alloc(GW_PROTO_SLE, 0x02, 0xA2, NULL);
    uint8_t b1 = gw_conn_alloc(GW_PROTO_BLE, 0x11, 0xB1, NULL);
    uint8_t b2 = gw_conn_alloc(GW_PROTO_BLE, 0x12, 0xB2, NULL);

    /* s1 <-> b1, s2 <-> b2 */
    gw_route_add(s1, b1); gw_route_add(b1, s1);
    gw_route_add(s2, b2); gw_route_add(b2, s2);

    /* s2 sends -- should reach b2, not b1 */
    uint8_t p2[] = {0x22, 0x22};
    gw_msg_t msg = {0};
    msg.type = GW_MSG_DATA_RECEIVED;
    msg.protocol = GW_PROTO_SLE;
    msg.proto_conn_id = 0x02;
    msg.data_len = sizeof(p2);
    memcpy(msg.data, p2, sizeof(p2));
    test_queue_inject(&msg);
    gw_service_process_events();

    const mock_send_t *bs = test_get_last_ble_send();
    TEST_ASSERT(bs->conn_id == 0x12);
    TEST_ASSERT(bs->data_len == sizeof(p2));
    TEST_ASSERT(memcmp(bs->data, p2, sizeof(p2)) == 0);
    (void)s1; (void)s2; (void)b1; (void)b2;
}

/* =========================================================================
 * Export all test functions
 * ====================================================================== */
void run_gateway_service_tests(void)
{
    printf("\n=== gateway_service tests ===\n");
    TEST_RUN(test_conn_alloc_find_free);
    TEST_RUN(test_conn_table_full);
    TEST_RUN(test_route_basic);
    TEST_RUN(test_event_connected);
    TEST_RUN(test_event_disconnected);
    TEST_RUN(test_event_data_sle_to_ble);
    TEST_RUN(test_event_data_ble_to_sle);
    TEST_RUN(test_data_unknown_conn);
    TEST_RUN(test_data_no_route);
    TEST_RUN(test_multi_conn_routing);
}
