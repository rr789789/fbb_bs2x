# SLE-BLE Gateway 上板实测 Bug 记录

> 记录上板测试过程中发现的运行时问题及修复方案（不含编译环境问题）。

---

## Bug 1: Gateway BLE adapter 连接时序竞态导致 SLE→BLE 转发失败

**文件**: `src/application/samples/products/sle_ble_gateway/gw_ble_adapter.c`

**现象**: `test.py` 中 SLE→BLE 方向测试失败，Board C 收不到数据；Board A 日志中可见 BLE 已连接，但无 `route:` 输出或 `gw_ble_adapter_send` 报错。

**根因**: BLE 客户端可能在 GATT service/characteristic 尚未完全建立前就完成连接。`gw_ble_connect_cbk` 在 `g_gw_ble_notification_att_hdl == 0` 时就将 `GW_MSG_CONN_CONNECTED` 发给 `gateway_service`，导致连接表里记录的 `attr_handle = 0`。后续 `gw_ble_adapter_send` 用 `attr_handle = 0` 调用 `gatts_notify_indicate`，协议栈返回失败，数据无法下发到 BLE client。

**修复方案**:
- 引入 `static volatile uint8_t g_gw_ble_conn_pending = 0;`
- 若连接到来时 `notification_att_hdl == 0`，设置 `conn_pending = 1` 并直接 return，**不**发送 `GW_MSG_CONN_CONNECTED`。
- 在 `gw_ble_char_add_cbk` 获得有效 `attr_handle` 后，检查 `conn_pending` 标志，若存在则补发 `GW_MSG_CONN_CONNECTED` 事件，携带正确的 `attr_handle`。

---

## Bug 2: BLE Client 未使能 Notification，导致无法接收 Gateway 下发的数据

**文件**: `src/application/samples/products/ble_uart/ble_uart_client/ble_uart_client.c`

**现象**: Gateway 侧 `gatts_notify_indicate` 返回成功，但 Board C 侧 UART 无输出，SLE→BLE 方向测试失败。

**根因**: `ble_uart_client.c` 在 GATT discovery 过程中发现 CCC descriptor (UUID 0x2902) 后，仅执行了 `gattc_read_req_by_uuid`，没有向 descriptor 写入 `0x01 0x00` 来使能 notification。因此 BLE 协议栈虽然收到了 server 的 notify，但不会把数据上报到应用层。

**修复方案**:
- 在 `ble_uart_client_discover_descriptor_cbk` 中检测 UUID 为 `0x2902` 的 descriptor。
- 发现后调用 `gattc_write_cmd` 向该 descriptor 写入 `[0x01, 0x00]`，开启 notification。
- 同时保存 `g_ble_uart_ccc_handle` 并打印日志确认使能成功。

---

## 验证结果

修复后重新编译 Board A 与 Board C 固件并烧录，运行：

```bash
python test.py --a COM8 --b COM5 --c COM4
```

结果：**全部 5 项检查通过**（SLE→BLE 单包、BLE→SLE 单包、SLE→BLE 3 连发包、双向并发）。
