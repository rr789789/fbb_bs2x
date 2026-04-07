/**
 * Mock gw_sle_adapter.h for PC-side unit tests.
 * The real adapter is not compiled in the test build; only the send stub
 * from mock_impl.c is linked.
 */
#ifndef MOCK_GW_SLE_ADAPTER_H
#define MOCK_GW_SLE_ADAPTER_H

#include <stdint.h>
#include "errcode.h"

int gw_sle_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                        const uint8_t *data, uint16_t len);

#endif /* MOCK_GW_SLE_ADAPTER_H */
