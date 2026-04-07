/**
 * Shared declarations for test helpers defined in mock_impl.c.
 */
#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdint.h>
#include "gateway_service.h"

/* Must match the definition in mock_impl.c exactly */
typedef struct {
    uint16_t conn_id;
    uint16_t attr_handle;
    uint8_t  data[512];
    uint16_t data_len;
    int      called;
} mock_send_t;

/* Queue helpers */
void test_queue_inject(const gw_msg_t *msg);
int  test_queue_depth(void);

/* Send capture helpers */
const mock_send_t *test_get_last_sle_send(void);
const mock_send_t *test_get_last_ble_send(void);
void test_reset_sends(void);

#endif /* TEST_HELPERS_H */
