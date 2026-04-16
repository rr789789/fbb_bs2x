#!/usr/bin/env python3
"""
hw_test/test.py — SLE-BLE Gateway automated hardware test
==========================================================
Verifies bidirectional data routing across the three-board setup:

  PC ──UART──► Board B (SLE client) ──SLE──► Board A (Gateway) ──BLE──► Board C (BLE client) ──UART──► PC
  PC ◄──UART── Board B (SLE client) ◄──SLE── Board A (Gateway) ◄──BLE── Board C (BLE client) ◄──UART── PC

Usage
-----
  pip install pyserial          # one-time

  python test.py --b COM3 --c COM5
  python test.py --b COM3 --c COM5 --a COM1      # Board A log port (optional, recommended)
  python test.py --b /dev/ttyUSB1 --c /dev/ttyUSB3 --baud 115200
  python test.py --b COM3 --c COM5 --timeout 20  # increase if wireless link is slow

Exit code: 0 = all passed, 1 = failure.
"""

import argparse
import sys
import time
import threading
import serial


# ---------------------------------------------------------------------------
# Serial reader (background thread)
# ---------------------------------------------------------------------------

class SerialReader:
    def __init__(self, port, baud, label=''):
        self.label = label
        self._buf  = bytearray()
        self._lock = threading.Lock()
        try:
            self.ser = serial.Serial(port, baud, timeout=0.5)
        except serial.SerialException as e:
            _die(f'Cannot open {port}: {e}')
        self._stop = threading.Event()
        self._t = threading.Thread(target=self._run, daemon=True, name=f'rx-{label}')
        self._t.start()

    def _run(self):
        while not self._stop.is_set():
            try:
                data = self.ser.read(512)
                if data:
                    with self._lock:
                        self._buf.extend(data)
            except serial.SerialException:
                break

    def flush_rx(self):
        with self._lock:
            self._buf.clear()

    def contains(self, pattern: bytes) -> bool:
        with self._lock:
            return pattern in bytes(self._buf)

    def wait_for(self, pattern: bytes, timeout: float) -> bool:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            if self.contains(pattern):
                return True
            time.sleep(0.05)
        return False

    def write(self, data: bytes):
        self.ser.write(data)

    def close(self):
        self._stop.set()
        self.ser.close()


# ---------------------------------------------------------------------------
# Output helpers
# ---------------------------------------------------------------------------

GREEN  = '\033[32m'
RED    = '\033[31m'
YELLOW = '\033[33m'
RESET  = '\033[0m'
PASS   = f'{GREEN}PASS{RESET}'
FAIL   = f'{RED}FAIL{RESET}'

_failures = 0

def _die(msg):
    print(f'{RED}[ERROR]{RESET} {msg}', file=sys.stderr)
    sys.exit(1)

def _log(msg):
    print(f'  {time.strftime("%H:%M:%S")}  {msg}')

def _check(label: str, ok: bool) -> bool:
    global _failures
    mark = PASS if ok else FAIL
    print(f'    [{mark}] {label}')
    if not ok:
        _failures += 1
    return ok


# ---------------------------------------------------------------------------
# Wait for wireless connections
# ---------------------------------------------------------------------------

def wait_for_route(board_a, timeout=40):
    """Wait until Gateway log shows both connections and route is established."""
    _log(f'Waiting up to {timeout}s for SLE+BLE connections & route ...')
    if board_a:
        # Watch for the gateway_demo route-established log line
        ok = board_a.wait_for(b'route: SLE', timeout)
        if ok:
            _log(f'{GREEN}Route established (seen in Board A log).{RESET}')
        else:
            _log(f'{YELLOW}Route log not seen — waiting extra 10s anyway ...{RESET}')
            time.sleep(10)
    else:
        _log('No Board A log port — waiting fixed 25s for wireless links to come up ...')
        time.sleep(25)


# ---------------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------------

# Non-ASCII sentinel bytes: these cannot appear in plain-text debug logs.

T1_PAYLOAD = bytes([0x01, 0x55, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x02, 0x03])
T2_PAYLOAD = bytes([0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00])
T3_BURST   = [bytes([0xA0 + i] * 4) for i in range(3)]
T4_B2C     = bytes([0xD1, 0xD2, 0xD3, 0xD4, 0xD5])
T4_C2B     = bytes([0xE1, 0xE2, 0xE3, 0xE4, 0xE5])


def test_sle_to_ble(bb, bc, timeout):
    print(f'\n  Test 1: SLE→BLE  (Board B sends → Board C receives)')
    bc.flush_rx()
    bb.write(T1_PAYLOAD)
    _log(f'Sent {T1_PAYLOAD.hex()} to Board B')
    _check('Payload arrived on Board C', bc.wait_for(T1_PAYLOAD, timeout))


def test_ble_to_sle(bb, bc, timeout):
    print(f'\n  Test 2: BLE→SLE  (Board C sends → Board B receives)')
    bb.flush_rx()
    bc.write(T2_PAYLOAD)
    _log(f'Sent {T2_PAYLOAD.hex()} to Board C')
    _check('Payload arrived on Board B', bb.wait_for(T2_PAYLOAD, timeout))


def test_multi_burst(bb, bc, timeout):
    print(f'\n  Test 3: Multi-packet burst SLE→BLE  (3 back-to-back)')
    bc.flush_rx()
    for p in T3_BURST:
        bb.write(p)
        time.sleep(0.05)
    _log(f'Sent 3 packets to Board B')
    ok = all(bc.wait_for(p, timeout) for p in T3_BURST)
    _check('All 3 packets arrived on Board C', ok)


def test_bidirectional(bb, bc, timeout):
    print(f'\n  Test 4: Bidirectional concurrent  (both boards send simultaneously)')
    bc.flush_rx()
    bb.flush_rx()
    bb.write(T4_B2C)
    bc.write(T4_C2B)
    _log(f'Sent simultaneously: B→C {T4_B2C.hex()},  C→B {T4_C2B.hex()}')
    _check('B→C: arrived on Board C', bc.wait_for(T4_B2C, timeout))
    _check('C→B: arrived on Board B', bb.wait_for(T4_C2B, timeout))


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description='SLE-BLE Gateway hardware test')
    parser.add_argument('--b',       required=True, metavar='PORT',
                        help='COM/tty port for Board B (SLE client UART0)')
    parser.add_argument('--c',       required=True, metavar='PORT',
                        help='COM/tty port for Board C (BLE client UART0)')
    parser.add_argument('--a',       default=None,  metavar='PORT',
                        help='COM/tty port for Board A log output (optional but recommended)')
    parser.add_argument('--baud',    type=int, default=115200,
                        help='UART baud rate for all ports (default 115200)')
    parser.add_argument('--timeout', type=float, default=12.0,
                        help='Per-packet receive timeout in seconds (default 12)')
    args = parser.parse_args()

    bar = '=' * 62
    print(f'\n{bar}')
    print(f'  SLE-BLE Gateway  ·  Hardware Automated Test')
    print(f'{bar}')
    print(f'  Board A (Gateway log) : {args.a or "(not connected — will use fixed wait)"}')
    print(f'  Board B (SLE client)  : {args.b}')
    print(f'  Board C (BLE client)  : {args.c}')
    print(f'  Baud rate             : {args.baud}')
    print(f'  Per-packet timeout    : {args.timeout} s')
    print(f'{bar}\n')

    ba = SerialReader(args.a, args.baud, 'A') if args.a else None
    bb = SerialReader(args.b, args.baud, 'B')
    bc = SerialReader(args.c, args.baud, 'C')

    try:
        wait_for_route(ba, timeout=40)

        test_sle_to_ble(bb, bc, args.timeout)
        time.sleep(0.3)
        test_ble_to_sle(bb, bc, args.timeout)
        time.sleep(0.3)
        test_multi_burst(bb, bc, args.timeout)
        time.sleep(0.3)
        test_bidirectional(bb, bc, args.timeout)

    finally:
        bb.close()
        bc.close()
        if ba:
            ba.close()

    print(f'\n{bar}')
    if _failures == 0:
        print(f'  Result : {PASS}  —  all 5 checks passed')
    else:
        print(f'  Result : {FAIL}  —  {_failures} check(s) failed')
    print(f'{bar}\n')
    sys.exit(0 if _failures == 0 else 1)


if __name__ == '__main__':
    main()
