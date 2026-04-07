/**
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * Description: Mock implementations for PC-side unit tests.
 *              Replaces: soc_osal, securec, osal_debug, adapter send functions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* =========================================================================
 * Bring in the real gateway_service.h so we share the same types.
 * The -I. flag ensures our local mock headers shadow the real ones.
 * ====================================================================== */
#include "gateway_service.h"

/* =========================================================================
 * Message queue mock
 * ====================================================================== */
#define MOCK_QUEUE_LEN  64

static struct {
    uint8_t  buf[MOCK_QUEUE_LEN][sizeof(gw_msg_t)];
    int      head;
    int      tail;
    int      count;
} g_mock_queue;

int osal_msg_queue_create(const char *name, unsigned long queue_len,
                          unsigned long *queue_id, unsigned int flags,
                          unsigned short max_msgsize)
{
    (void)name; (void)queue_len; (void)flags; (void)max_msgsize;
    memset(&g_mock_queue, 0, sizeof(g_mock_queue));
    *queue_id = 1;
    return 0; /* OSAL_SUCCESS */
}

int osal_msg_queue_write_copy(unsigned long queue_id, void *buffer,
                              unsigned int bufsize, unsigned int timeout)
{
    (void)queue_id; (void)timeout;
    if (g_mock_queue.count >= MOCK_QUEUE_LEN) {
        return 1; /* OSAL_FAILURE */
    }
    size_t copy = (bufsize < sizeof(gw_msg_t)) ? bufsize : sizeof(gw_msg_t);
    memcpy(g_mock_queue.buf[g_mock_queue.tail], buffer, copy);
    g_mock_queue.tail = (g_mock_queue.tail + 1) % MOCK_QUEUE_LEN;
    g_mock_queue.count++;
    return 0; /* OSAL_SUCCESS */
}

int osal_msg_queue_read_copy(unsigned long queue_id, void *buffer,
                             unsigned int *bufsize, unsigned int timeout)
{
    (void)queue_id; (void)timeout;
    if (g_mock_queue.count == 0) {
        return 1; /* OSAL_FAILURE */
    }
    size_t copy = (*bufsize < sizeof(gw_msg_t)) ? *bufsize : sizeof(gw_msg_t);
    memcpy(buffer, g_mock_queue.buf[g_mock_queue.head], copy);
    g_mock_queue.head = (g_mock_queue.head + 1) % MOCK_QUEUE_LEN;
    g_mock_queue.count--;
    return 0; /* OSAL_SUCCESS */
}

int osal_msg_queue_delete(unsigned long queue_id)
{
    (void)queue_id;
    return 0;
}

/* Test helper: inject a message as if it came from a protocol callback */
void test_queue_inject(const gw_msg_t *msg)
{
    osal_msg_queue_write_copy(1, (void *)msg, sizeof(*msg), 0);
}

int test_queue_depth(void)
{
    return g_mock_queue.count;
}

/* =========================================================================
 * securec mock
 * ====================================================================== */
int memcpy_s(void *dest, size_t dest_size, const void *src, size_t count)
{
    if (count > dest_size) {
        return 1; /* ERANGE */
    }
    memcpy(dest, src, count);
    return 0; /* EOK */
}

int memset_s(void *dest, size_t dest_size, int ch, size_t count)
{
    if (count > dest_size) {
        return 1;
    }
    memset(dest, ch, count);
    return 0;
}

/* =========================================================================
 * osal_debug mock
 * ====================================================================== */
void osal_printk(const char *fmt, ...)
{
    /* Suppress output during tests; uncomment for verbose mode */
    (void)fmt;
}

/* =========================================================================
 * Adapter send mocks -- capture last sent frame for assertions
 * ====================================================================== */
typedef struct {
    uint16_t conn_id;
    uint16_t attr_handle;
    uint8_t  data[512];
    uint16_t data_len;
    int      called;
} mock_send_t;

static mock_send_t g_last_sle_send;
static mock_send_t g_last_ble_send;

int gw_sle_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                        const uint8_t *data, uint16_t len)
{
    g_last_sle_send.conn_id    = conn_id;
    g_last_sle_send.attr_handle = attr_handle;
    g_last_sle_send.data_len   = len;
    if (len > 0 && data != NULL) {
        size_t cp = len < sizeof(g_last_sle_send.data) ? len : sizeof(g_last_sle_send.data);
        memcpy(g_last_sle_send.data, data, cp);
    }
    g_last_sle_send.called++;
    return 0; /* ERRCODE_SUCC */
}

int gw_ble_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                        const uint8_t *data, uint16_t len)
{
    g_last_ble_send.conn_id    = conn_id;
    g_last_ble_send.attr_handle = attr_handle;
    g_last_ble_send.data_len   = len;
    if (len > 0 && data != NULL) {
        size_t cp = len < sizeof(g_last_ble_send.data) ? len : sizeof(g_last_ble_send.data);
        memcpy(g_last_ble_send.data, data, cp);
    }
    g_last_ble_send.called++;
    return 0; /* ERRCODE_SUCC */
}

/* Test helpers to query captured sends */
const mock_send_t *test_get_last_sle_send(void) { return &g_last_sle_send; }
const mock_send_t *test_get_last_ble_send(void) { return &g_last_ble_send; }

void test_reset_sends(void)
{
    memset(&g_last_sle_send, 0, sizeof(g_last_sle_send));
    memset(&g_last_ble_send, 0, sizeof(g_last_ble_send));
}
