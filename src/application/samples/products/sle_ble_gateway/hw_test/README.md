# SLE-BLE Gateway 硬件测试指南

## 你需要准备

| 物品 | 数量 | 说明 |
|------|------|------|
| BS21E 开发板 | 3 块 | 型号相同即可 |
| USB 线 | 3 根 | 每块板接一根到 PC |

**板与板之间无任何有线连接，全部无线通信。**

---

## 操作步骤（共 3 步）

### 第 1 步：安装 Python 依赖（只做一次）

```
pip install pyserial
```

---

### 第 2 步：编译三块板的固件

在 **SDK 根目录**（含 `src/` 的那层目录）打开命令行，执行：

```
python src/application/samples/products/sle_ble_gateway/hw_test/build.py
```

编译完成后，固件自动存放到：
```
hw_test/firmware/board_a_gateway/application.fwpkg   ← Board A 烧录用
hw_test/firmware/board_b_sle_client/application.fwpkg ← Board B 烧录用
hw_test/firmware/board_c_ble_client/application.fwpkg ← Board C 烧录用
```

> Board C 通过扫描广播名 `ble_gw` 自动找到 Board A 并连接，无需任何 MAC 地址配置。

---

### 第 3 步：烧录所有板子并运行自动化测试

#### 3.1 烧录

| 板子 | 固件文件 |
|------|---------|
| **Board A** | `firmware/board_a_gateway/application.fwpkg` |
| **Board B** | `firmware/board_b_sle_client/application.fwpkg` |
| **Board C** | `firmware/board_c_ble_client/application.fwpkg` |

#### 3.2 查看串口号

三块板全部用 USB 连接 PC。打开 **设备管理器 → 端口**，记下三块板对应的 COM 号（例如 COM1、COM3、COM5）。

#### 3.3 运行测试

```
python src/application/samples/products/sle_ble_gateway/hw_test/test.py ^
    --a COM1 --b COM3 --c COM5
```

> - `--a` = Board A 的串口（可选，用于检测路由建立）
> - `--b` = Board B 的串口（SLE 客户端）
> - `--c` = Board C 的串口（BLE 客户端）

#### 3.4 预期结果

连接建立需要约 10–20 秒，然后自动运行 4 项测试：

```
==============================================================
  SLE-BLE Gateway  ·  Hardware Automated Test
==============================================================
  ...
  10:00:02  Route established (seen in Board A log).

  Test 1: SLE→BLE  (Board B sends → Board C receives)
    [PASS] Payload arrived on Board C

  Test 2: BLE→SLE  (Board C sends → Board B receives)
    [PASS] Payload arrived on Board B

  Test 3: Multi-packet burst SLE→BLE  (3 back-to-back)
    [PASS] All 3 packets arrived on Board C

  Test 4: Bidirectional concurrent  (both boards send simultaneously)
    [PASS] B→C: arrived on Board C
    [PASS] C→B: arrived on Board B

==============================================================
  Result : PASS  —  all 5 checks passed
==============================================================
```

---

## 目录结构

```
hw_test/
├── README.md          ← 你正在看的文件
├── build.py           ← 编译所有固件
├── test.py            ← 自动化测试
├── requirements.txt   ← Python 依赖
└── firmware/
    ├── board_a_gateway/       ← Board A 固件
    ├── board_b_sle_client/    ← Board B 固件
    └── board_c_ble_client/    ← Board C 固件
```

---

## 三块板的角色说明

```
Board B (SLE客户端)
     │  SLE 无线
     ▼
Board A (网关) ← 自动把 SLE 连接和 BLE 连接桥接起来
     │  BLE 无线
     ▼
Board C (BLE客户端)
```

Board A 不需要任何操作——两侧设备连上后，网关自动建立双向路由。

Board B 扫描广播名 `sle_uart_server` 后连接 Board A。
Board C 扫描广播名 `ble_gw` 后连接 Board A。
两者均无需 MAC 地址，开箱即用。

---

## 常见问题

| 现象 | 原因 | 处理 |
|------|------|------|
| Board C 扫描不到 Board A | Board A 未正常启动，或 BLE 广播未开始 | 重新上电 Board A，观察串口日志是否有 `[gw_ble] adapter init ok` |
| Board B 连接超时 | Board A 未正常启动 | 重新上电 Board A，观察串口日志是否有 `[gw_sle] adapter init ok` |
| 测试超时未收到数据 | 两侧未同时连接，路由未建立 | 加大 `--timeout 30` 重试 |
| 串口乱码 | 波特率不对 | 加 `--baud 9600` 等实际波特率 |
| build.py 报"SDK not found" | 在错误目录执行 | 必须在 SDK 根目录（含 `src/` 的那层）执行 |
