# 硬件测试执行计划（给未来的我）

> 用户已搭好三板硬件环境，我来执行测试、定位问题、修复代码，直到全部通过。
> 本文档是我的行动指南，不是给用户看的操作手册（用户看 README.md）。

---

## 环境概况（回顾）

| 板子 | 固件 | 关键配置 |
|------|------|---------|
| Board A (Gateway) | `sle_ble_gateway` | SLE adv name=`"sle_uart_server"`, BLE char UUID=`0xEFEF` |
| Board B (SLE Client) | `sle_uart CLIENT` | 扫描设备名 `"sle_uart_server"` 后连接，UART0 收发数据 |
| Board C (BLE Client) | `ble_uart CLIENT` | 按 MAC 连接 Board A，寻找 char UUID `0xEFEF`，UART0 收发数据 |

关键代码文件：
- `sle_ble_gateway/gateway_service.c` — 连接表、路由、队列主循环
- `sle_ble_gateway/gateway_demo.c` — 自动路由（首个 SLE ↔ 首个 BLE）
- `sle_ble_gateway/gw_sle_adapter.c` — SLE SSAP Server，adv name 在第 57 行
- `sle_ble_gateway/gw_ble_adapter.c` — BLE GATT Server，char UUID 在第 31 行
- `ble_uart/ble_uart_client/ble_uart_client.c` — Board C MAC 在第 39 行

测试脚本：`hw_test/test.py`
编译脚本：`hw_test/build.py`

---

## 第一步：用户回来时，我要问的信息

用户找我时，立刻收集以下信息（如果他们没主动提供）：

1. **三块板的 COM 口** —— `--a`, `--b`, `--c` 各对应哪个 COM
2. **固件是否已全部烧录** —— 特别是 Board C（需要 MAC）
3. **Board A 串口是否有输出** —— 确认固件正常启动
4. **上次测试输出（如有）** —— 直接粘贴，我来分析

---

## 第二步：测试执行顺序

```
1. 确认环境（30秒）
2. 运行 test.py，观察完整输出
3. 根据输出进入对应诊断分支
4. 修复 → rebuild（build.py --only a/b/c）→ 重新烧录 → 重新测试
5. 循环直到全部 PASS
```

### 2.1 运行测试

```bash
python hw_test/test.py --a COM_A --b COM_B --c COM_C --timeout 20
```

如果连接非常慢（SLE 配对需时），加 `--timeout 30`。

---

## 第三步：故障诊断手册

### 故障 A：等待路由超时，连接从未建立

**现象**：`wait_for_route` 超时，Board A log 无 `"Route: SLE"` 输出

**诊断步骤**：
1. 看 Board A 串口：有没有 `[gw_sle] sle enabled` 和 `[gw_ble] ble enabled status=1`？
   - 没有 → Gateway 初始化失败，检查 `gateway_entry.c` 的 `app_run` 注册，检查 SLE/BLE 初始化回调链
2. 看 Board B 串口：有没有 `sle uart scan data :sle_uart_server`？
   - 没有 → Board B 扫描不到 Gateway。检查 `gw_sle_adapter.c` 广播名是否为 `"sle_uart_server"`（第 57 行）
3. 看 Board C 串口：有没有 `connect state change conn_id: 0`？
   - 没有 → Board C 扫描不到 Gateway。检查 `ble_uart_client.c` 的 `adv_data_has_name()` 里目标名是否为 `"ble_gw"`，以及 `gw_ble_adapter.c` 的 `g_gw_ble_local_name` 是否一致
4. 两侧都连上了但没有 `Route` log？
   - 检查 `gateway_demo.c`：`gateway_demo_on_connect` 是否被正确注册到 `gw_service_init(on_connect, on_disconnect)`
   - 检查 `gateway_entry.c` 中 `gw_service_init` 调用是否传入了 `gateway_demo_on_connect` 和 `gateway_demo_on_disconnect`

**可能的修复**：
- SLE 广播名不匹配 → `gw_sle_adapter.c:57` 改 `g_gw_sle_adv_name`
- Board C 扫描名不匹配 → 确认 `ble_uart_client.c` 中 `BLE_GW_SERVER_NAME = "ble_gw"` 与 `gw_ble_adapter.c` 中 `g_gw_ble_local_name` 一致
- Demo 回调未注册 → 检查 `gateway_entry.c` 的 `gw_service_init(...)` 参数

---

### 故障 B：路由建立了，但 Test 1（SLE→BLE）FAIL

**现象**：Board A log 有 `Route: SLE`，但 Board C 没收到数据

**诊断步骤**：
1. Board A log 有没有 `[gw_ble] send` 相关输出（或错误）？
   - 有 `send fail` → `gw_ble_adapter_send` 返回错误，检查 BLE notification attr handle 是否正确
   - 没有任何 send 相关 → 数据没到 `gw_handle_data`，检查 SLE write callback → `gw_service_post_msg` → 队列读取路径
2. Board A log 有没有 `[gw_sle]` 的 write 相关输出？
   - 没有 → Board B 写入数据了吗？检查 `sle_uart_client` 是否在 UART 收到字节后真的发了 SLE write
3. `gw_route_lookup` 返回了正确 slot 吗？
   - 在 `gateway_service.c` 的 `gw_handle_data` 中加 `osal_printk` 打印 src_slot、dst_slot

**可能的修复**：
- `g_gw_ble_notification_att_hdl` 为 0 → BLE 服务构建回调链未完成（`gw_ble_char_add_cbk` 没被调用）
  - 检查 `gw_ble_service_add_cbk` 里的 UUID 比较逻辑，确认 `GW_BLE_UUID_SERVICE=0xAA01` 匹配
- `gw_conn_get(dst_slot)->attr_handle` 为 0 → 连接时没有存 attr_handle
  - 检查 `gw_ble_connect_cbk` 中 `msg.attr_handle = g_gw_ble_notification_att_hdl`（要在服务构建完成后）
  - **关键时序**：`gatts_add_service` 的 callback chain 是异步的。`gw_ble_adapter_init` 在 `enable_ble` 后调 `gatts_add_service`，但 char handle 在 `gw_ble_char_add_cbk` 中才被赋值。如果 BLE 设备在服务构建完成前就连接了，`attr_handle` 会是 0。
  - 修复：在 `gw_ble_service_start_cbk` 中设置一个 `g_gw_ble_service_ready` flag，连接时检查这个 flag

---

### 故障 C：SLE→BLE PASS，但 BLE→SLE FAIL

**诊断步骤**：
1. Board A log 有没有 `[gw_ble]` 的 write callback 输出？
   - 没有 → BLE client（Board C）没有向 Gateway 发送 GATT write
   - 检查 Board C 的 `ble_uart_client` 是否正确发现了 Gateway 的 characteristic，并且向正确的 handle 执行 write
2. Board A 收到了数据但没有转发到 Board B？
   - 检查 `gw_conn_find_by_id(GW_PROTO_BLE, conn_id)` 是否能找到 BLE 连接
   - 检查 BLE→SLE 的路由：`gw_route_lookup(b_slot)` 应返回 `s_slot`

---

### 故障 D：Test 3（burst）或 Test 4（concurrent）FAIL，但 Test 1/2 PASS

**现象**：单包通过，多包或并发失败

**诊断步骤**：
1. 是否有数据丢包？（部分 payload 收到，部分没有）
   - OSAL 队列满了（`gw_service_post_msg` 返回失败，但当前代码没有检查返回值）
   - 修复：加大队列深度（`gw_service_init` 中 `osal_msg_queue_create` 的 queue_len 从 16 改为 32）
2. 是否有串口字节丢失（物理层）？
   - 加大 `test.py` 中的包间隔（`time.sleep(0.1)`）
3. 并发时双向数据互相干扰？
   - 检查 `gateway_service.c` 是否单线程处理（应该是，单任务 + 单队列是安全的）

---

### 故障 E：构建失败

**诊断步骤**：
1. `build.py` 在 config 修改后失败 → 查看 Kconfig 依赖
   - `CONFIG_SAMPLE_SUPPORT_SLE_BLE_GATEWAY=y` 依赖 `CONFIG_ENABLE_PRODUCTS_SAMPLE=y` 依赖 `CONFIG_SAMPLE_ENABLE=y`
   - `build.py` 已经全部注入，但如果 Kconfig 有其他依赖，查看构建日志
2. 编译报错找不到头文件
   - 检查 `sle_ble_gateway/CMakeLists.txt` 中的 `PUBLIC_HEADER` 路径
3. 链接错误（未定义的符号）
   - 检查 `gw_sle_adapter.c` 和 `gw_ble_adapter.c` 是否都在 `SOURCES_LIST` 中

---

## 第四步：修复后的重新构建流程

```bash
# 只重建 Board A（最常改的板子）
python hw_test/build.py --only a

# 用户重新烧录 Board A 后，重新运行测试
python hw_test/test.py --a COM_A --b COM_B --c COM_C --timeout 20

# 如果 Board C 的 MAC 需要更新，重建 Board C
python hw_test/build.py --board-c-mac AA:BB:CC:DD:EE:FF --only c
```

**重建不影响工作区**：`build.py` 会自动备份/恢复 `standard_bs21e_1100e.config` 和 `ble_uart_client.c`。

---

## 第五步：成功标准

`test.py` 输出最后一行为：
```
Result : PASS  —  all 5 checks passed
```

所有 5 个 check 全部通过后，测试完成。

---

## 附：关键代码位置速查

| 需要改什么 | 文件 | 行号/位置 |
|-----------|------|---------|
| SLE 广播名 | `gw_sle_adapter.c` | 第 57 行 `g_gw_sle_adv_name` |
| BLE char UUID | `gw_ble_adapter.c` | 第 31 行 `GW_BLE_UUID_CHAR` |
| BLE 服务 UUID | `gw_ble_adapter.c` | 第 30 行 `GW_BLE_UUID_SERVICE` |
| Board C 目标设备名 | `ble_uart/ble_uart_client/ble_uart_client.c` | `BLE_GW_SERVER_NAME` 宏（扫描名匹配） |
| 路由建立逻辑 | `gateway_demo.c` | `gateway_demo_on_connect()` |
| 数据转发逻辑 | `gateway_service.c` | `gw_handle_data()` |
| 队列深度 | `gateway_service.c` | `gw_service_init()` 中的 `queue_len` 参数 |
| BLE 连接时存 attr_handle | `gw_ble_adapter.c` | `gw_ble_connect_cbk()` |
