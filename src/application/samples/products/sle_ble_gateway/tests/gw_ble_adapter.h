/**
 * Mock gw_ble_adapter.h for PC-side unit tests.
 */
#ifndef MOCK_GW_BLE_ADAPTER_H
#define MOCK_GW_BLE_ADAPTER_H

#include <stdint.h>
#include "errcode.h"

int gw_ble_adapter_send(uint16_t conn_id, uint16_t attr_handle,
                        const uint8_t *data, uint16_t len);

#endif /* MOCK_GW_BLE_ADAPTER_H */
