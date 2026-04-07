#!/usr/bin/env python3
"""
hw_test/build.py — SLE-BLE Gateway firmware builder
====================================================
Builds firmware for all three boards and places the .fwpkg files in:
  firmware/board_a_gateway/
  firmware/board_b_sle_client/
  firmware/board_c_ble_client/

Usage
-----
  # Step 1: build Board A (gateway) + Board B (SLE client)
  python build.py

  # Step 2: after reading Board A's BLE MAC from phone, build Board C
  python build.py --board-c-mac AA:BB:CC:DD:EE:FF

  # Build everything in one shot (if you already know the MAC)
  python build.py --board-c-mac AA:BB:CC:DD:EE:FF --all
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
import time

# ---------------------------------------------------------------------------
# Paths (all relative to the SDK repo root)
# ---------------------------------------------------------------------------

SCRIPT_DIR  = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT   = os.path.normpath(os.path.join(SCRIPT_DIR, *(['..'] * 6)))
BUILD_PY    = os.path.join(REPO_ROOT, 'src', 'build.py')
TARGET      = 'standard-bs21e-1100e'

CONFIG_FILE = os.path.join(
    REPO_ROOT, 'src', 'build', 'config', 'target_config',
    'bs21e', 'menuconfig', 'acore', 'standard_bs21e_1100e.config'
)

BLE_CLIENT_C = os.path.join(
    REPO_ROOT, 'src', 'application', 'samples', 'products',
    'ble_uart', 'ble_uart_client', 'ble_uart_client.c'
)

BUILD_OUTPUT = os.path.join(
    REPO_ROOT, 'src', 'output', 'bs21e', 'fwpkg', TARGET, 'bs21e_all_in_one.fwpkg'
)

FIRMWARE_DIR = os.path.join(SCRIPT_DIR, 'firmware')

# ---------------------------------------------------------------------------
# Config keys to strip before injecting per-board settings
# (these lines are replaced, not accumulated)
# ---------------------------------------------------------------------------

SAMPLE_KEYS = [
    'CONFIG_SAMPLE_ENABLE',
    'CONFIG_ENABLE_PRODUCTS_SAMPLE',
    'CONFIG_SAMPLE_SUPPORT_SLE_UART',
    'CONFIG_SAMPLE_SUPPORT_SLE_UART_SERVER',
    'CONFIG_SAMPLE_SUPPORT_SLE_UART_CLIENT',
    'CONFIG_SAMPLE_SUPPORT_NORMAL_TYPE',
    'CONFIG_SAMPLE_SUPPORT_LOW_LATENCY_TYPE',
    'CONFIG_SLE_UART_BUS',
    'CONFIG_SAMPLE_SUPPORT_BLE_UART',
    'CONFIG_SAMPLE_SUPPORT_BLE_UART_SERVER',
    'CONFIG_SAMPLE_SUPPORT_BLE_UART_CLIENT',
    'CONFIG_BLE_UART_BUS',
    'CONFIG_SAMPLE_SUPPORT_SLE_BLE_GATEWAY',
    'CONFIG_GW_MAX_CONNECTIONS',
    'CONFIG_GW_DATA_MAX_LEN',
]

# ---------------------------------------------------------------------------
# Per-board config injections
# ---------------------------------------------------------------------------

BOARD_A_SETTINGS = """\
# --- SLE-BLE Gateway (Board A) ---
CONFIG_SAMPLE_ENABLE=y
CONFIG_ENABLE_PRODUCTS_SAMPLE=y
CONFIG_SAMPLE_SUPPORT_SLE_BLE_GATEWAY=y
CONFIG_GW_MAX_CONNECTIONS=8
CONFIG_GW_DATA_MAX_LEN=256
"""

BOARD_B_SETTINGS = """\
# --- SLE UART Client (Board B) ---
CONFIG_SAMPLE_ENABLE=y
CONFIG_ENABLE_PRODUCTS_SAMPLE=y
CONFIG_SAMPLE_SUPPORT_SLE_UART=y
CONFIG_SAMPLE_SUPPORT_SLE_UART_CLIENT=y
CONFIG_SAMPLE_SUPPORT_NORMAL_TYPE=y
CONFIG_SLE_UART_BUS=0
"""

BOARD_C_SETTINGS = """\
# --- BLE UART Client (Board C) ---
CONFIG_SAMPLE_ENABLE=y
CONFIG_ENABLE_PRODUCTS_SAMPLE=y
CONFIG_SAMPLE_SUPPORT_BLE_UART=y
CONFIG_SAMPLE_SUPPORT_BLE_UART_CLIENT=y
CONFIG_BLE_UART_BUS=0
"""


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def die(msg):
    print(f'\n[ERROR] {msg}', file=sys.stderr)
    sys.exit(1)


def info(msg):
    print(f'  {msg}')


def section(title):
    bar = '=' * 60
    print(f'\n{bar}\n  {title}\n{bar}')


def patch_config(extra_settings: str):
    """Strip all existing sample keys from the config file, append new ones."""
    with open(CONFIG_FILE, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    out = []
    for line in lines:
        stripped = line.strip()
        # Drop lines that define (or comment-out) a sample key
        key_hit = any(
            stripped.startswith(f'CONFIG_{k.split("CONFIG_")[1]}') or
            stripped.startswith(f'# CONFIG_{k.split("CONFIG_")[1]} ')
            for k in SAMPLE_KEYS
            if 'CONFIG_' in k
        )
        # Simpler: check by splitting on '='
        key_name = stripped.lstrip('# ').split('=')[0].split(' ')[0]
        if key_name in SAMPLE_KEYS:
            continue
        out.append(line)

    # Remove trailing blank lines, then append our block
    while out and out[-1].strip() == '':
        out.pop()
    out.append('\n')
    out.append(extra_settings)

    with open(CONFIG_FILE, 'w', encoding='utf-8') as f:
        f.writelines(out)


def restore_config(backup: str):
    shutil.copy2(backup, CONFIG_FILE)
    info('Config restored.')


def backup_config() -> str:
    ts  = time.strftime('%Y%m%d_%H%M%S')
    bak = CONFIG_FILE + f'.bak_{ts}'
    shutil.copy2(CONFIG_FILE, bak)
    info(f'Config backed up → {os.path.basename(bak)}')
    return bak


def _wsl_path(win_path: str) -> str:
    """Convert Windows path to WSL /mnt/... path."""
    p = win_path.replace('\\', '/')
    if len(p) >= 2 and p[1] == ':':
        drive = p[0].lower()
        p = '/mnt/' + drive + p[2:]
    return p


def _in_wsl() -> bool:
    """Return True if this script is running inside a WSL environment."""
    return os.path.exists('/proc/version') and 'microsoft' in open('/proc/version').read().lower()


def run_build():
    """Run the SDK build inside WSL fbb_ws63 where cmake + toolchain live."""
    if _in_wsl():
        # Already inside WSL — call the build script directly
        wsl_build = _wsl_path(BUILD_PY)
        wsl_cwd   = _wsl_path(REPO_ROOT)
        info(f'Running (WSL-native): python3 {wsl_build} -c {TARGET}')
        cmd = ['python3', wsl_build, '-c', TARGET]
        result = subprocess.run(cmd, cwd=wsl_cwd)
    else:
        wsl_build = _wsl_path(BUILD_PY)
        info(f'Running: wsl -d fbb_ws63 -- python3 {wsl_build} -c {TARGET}')
        cmd = ['wsl', '-d', 'fbb_ws63', '--', 'python3', wsl_build, '-c', TARGET]
        result = subprocess.run(cmd, cwd=REPO_ROOT)
    if result.returncode != 0:
        die(f'Build failed (exit {result.returncode}). Check the output above.')


def copy_firmware(dest_subdir: str):
    if not os.path.isfile(BUILD_OUTPUT):
        die(f'Expected firmware not found: {BUILD_OUTPUT}')
    dest_dir = os.path.join(FIRMWARE_DIR, dest_subdir)
    os.makedirs(dest_dir, exist_ok=True)
    dest = os.path.join(dest_dir, 'application.fwpkg')
    shutil.copy2(BUILD_OUTPUT, dest)
    info(f'Firmware copied → {os.path.relpath(dest, SCRIPT_DIR)}')


# ---------------------------------------------------------------------------
# BLE MAC patching for Board C
# ---------------------------------------------------------------------------

MAC_RE = re.compile(
    r'(static\s+uint8_t\s+g_ble_server_addr_connect\s*\[\s*\]\s*=\s*\{)[^}]+(})',
    re.DOTALL
)

def mac_str_to_c_array(mac_str: str) -> str:
    """Convert 'AA:BB:CC:DD:EE:FF' → '{ 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA }' (little-endian)."""
    parts = mac_str.strip().split(':')
    if len(parts) != 6:
        die(f'Invalid MAC address format: {mac_str!r}  (expected AA:BB:CC:DD:EE:FF)')
    try:
        bytes_be = [int(p, 16) for p in parts]
    except ValueError:
        die(f'Invalid MAC address hex values: {mac_str!r}')
    bytes_le = list(reversed(bytes_be))
    return ', '.join(f'0x{b:02X}' for b in bytes_le)


def patch_ble_client_mac(mac_str: str):
    with open(BLE_CLIENT_C, 'r', encoding='utf-8') as f:
        src = f.read()

    c_array = mac_str_to_c_array(mac_str)
    replacement = rf'\g<1> {c_array} \g<2>'
    new_src, n = MAC_RE.subn(replacement, src)

    if n == 0:
        die(f'Could not find g_ble_server_addr_connect[] in {BLE_CLIENT_C}')

    with open(BLE_CLIENT_C, 'w', encoding='utf-8') as f:
        f.write(new_src)
    info(f'BLE client MAC patched → {c_array}')


def restore_ble_client_mac():
    with open(BLE_CLIENT_C, 'r', encoding='utf-8') as f:
        src = f.read()
    replacement = r'\g<1> 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 \g<2>'
    new_src, _ = MAC_RE.subn(replacement, src)
    with open(BLE_CLIENT_C, 'w', encoding='utf-8') as f:
        f.write(new_src)
    info('BLE client MAC restored to default placeholder.')


# ---------------------------------------------------------------------------
# Board build functions
# ---------------------------------------------------------------------------

def build_board_a():
    section('Building Board A — Gateway (sle_ble_gateway)')
    backup = backup_config()
    try:
        patch_config(BOARD_A_SETTINGS)
        run_build()
        copy_firmware('board_a_gateway')
    finally:
        restore_config(backup)
    print('  [OK] Board A firmware ready.')


def build_board_b():
    section('Building Board B — SLE Client (sle_uart_client)')
    backup = backup_config()
    try:
        patch_config(BOARD_B_SETTINGS)
        run_build()
        copy_firmware('board_b_sle_client')
    finally:
        restore_config(backup)
    print('  [OK] Board B firmware ready.')


def build_board_c():
    section('Building Board C — BLE Client (ble_uart_client)')
    backup = backup_config()
    try:
        patch_config(BOARD_C_SETTINGS)
        run_build()
        copy_firmware('board_c_ble_client')
    finally:
        restore_config(backup)
    print('  [OK] Board C firmware ready.')


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description='Build SLE-BLE Gateway hardware test firmwares')
    parser.add_argument('--only', choices=['a', 'b', 'c'],
                        help='Build only one specific board')
    args = parser.parse_args()

    if not os.path.isfile(BUILD_PY):
        die(f'SDK build script not found at expected path:\n  {BUILD_PY}\n'
            'Make sure you are running this from inside the SDK repo.')

    if args.only == 'a':
        build_board_a()
    elif args.only == 'b':
        build_board_b()
    elif args.only == 'c':
        build_board_c()
    else:
        build_board_a()
        build_board_b()
        build_board_c()

    section('Firmware locations')
    for sub in ['board_a_gateway', 'board_b_sle_client', 'board_c_ble_client']:
        fwpkg = os.path.join(FIRMWARE_DIR, sub, 'application.fwpkg')
        status = 'READY' if os.path.isfile(fwpkg) else 'not built yet'
        print(f'  [{status:^12s}]  firmware/{sub}/application.fwpkg')


if __name__ == '__main__':
    main()
