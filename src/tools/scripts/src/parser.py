#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LST 文件解析模块

职责:
    读取 lst 文件，提取所有 jal/jalr 跳转指令，
    输出标准化的 JumpInstruction 对象列表。

设计原则:
    - 单一职责: 只做解析，不做分析或报告
    - 无副作用: 不修改外部状态，纯输入输出
    - 容错性: 忽略不认识的行，不影响整体解析
"""

import re
from typing import List

from .models import JumpInstruction


class JumpParser:
    """跳转指令解析器"""

    # 函数标签行: 9013bd68 <evt_task_gle_scan_parse_rr_ptr>:
    _FUNC_LABEL_RE = re.compile(r'^([0-9a-fA-F]+)\s+<([^>]+)>:$')

    # 函数名行: evt_task_gle_scan_parse_rr_ptr():
    _FUNC_NAME_RE = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*)\(\):$')

    # 指令行: 9013bd78:\t63a050ef          \tjal\tra,901413b2 <func>
    _INSN_LINE_RE = re.compile(
        r'^([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+(.*)$'
    )

    # jal 格式: jal ra,901413b2 <func>
    _JAL_RE = re.compile(
        r'^jal\s+(\w+),\s*([0-9a-fA-F]+)(?:\s+<([^>]+)>)?'
    )

    # jalr 格式1: jalr offset(reg) # addr <func>
    _JALR_TARGET_RE = re.compile(
        r'^jalr\s+([-\d]+)\((\w+)\)\s*#\s*([0-9a-fA-F]+)(?:\s+<([^>]+)>)?'
    )

    # jalr 格式2: jalr reg  (无偏移，无目标地址信息)
    _JALR_REG_RE = re.compile(r'^jalr\s+(\w+)$')

    # jalr 格式3: jalr offset(reg)  没有 # 后面的目标地址
    _JALR_OFFSET_RE = re.compile(
        r'^jalr\s+([-\d]+)\((\w+)\)$'
    )

    def __init__(self):
        self.current_func: str = 'UNKNOWN'

    def parse_file(self, lst_path: str) -> List[JumpInstruction]:
        """
        解析 lst 文件，返回所有跳转指令列表。

        Args:
            lst_path: lst 文件绝对路径

        Returns:
            JumpInstruction 对象列表
        """
        results: List[JumpInstruction] = []
        self.current_func = 'UNKNOWN'

        with open(lst_path, 'r', encoding='utf-8', errors='ignore') as f:
            for line in f:
                instr = self._parse_line(line.rstrip('\n\r'))
                if instr:
                    results.append(instr)

        return results

    def _parse_line(self, line: str) -> JumpInstruction | None:
        """解析单行，返回 JumpInstruction 或 None"""
        # 尝试匹配函数标签
        m = self._FUNC_LABEL_RE.match(line)
        if m:
            self.current_func = m.group(2)
            return None

        # 尝试匹配函数名行
        m = self._FUNC_NAME_RE.match(line)
        if m:
            self.current_func = m.group(1)
            return None

        # 尝试匹配指令行
        m = self._INSN_LINE_RE.match(line)
        if not m:
            return None

        addr_str, hexcode, insn_part = m.groups()
        return self._parse_instruction(addr_str, hexcode, insn_part, line)

    def _parse_instruction(self, addr: str, hexcode: str,
                           insn: str, raw: str) -> JumpInstruction | None:
        """解析具体指令部分"""
        # jal
        jal_m = self._JAL_RE.match(insn)
        if jal_m:
            reg, target_addr, target_func = jal_m.groups()
            return JumpInstruction(
                caller_addr=addr,
                caller_func=self.current_func,
                insn_type='jal',
                insn_hex=hexcode,
                register=reg,
                target_addr=target_addr,
                target_func=target_func,
                raw_line=raw,
            )

        # jalr with target
        jalr_t_m = self._JALR_TARGET_RE.match(insn)
        if jalr_t_m:
            offset, reg, target_addr, target_func = jalr_t_m.groups()
            return JumpInstruction(
                caller_addr=addr,
                caller_func=self.current_func,
                insn_type='jalr',
                insn_hex=hexcode,
                register=reg,
                target_addr=target_addr,
                target_func=target_func,
                raw_line=raw,
            )

        # jalr offset(reg) without target
        jalr_o_m = self._JALR_OFFSET_RE.match(insn)
        if jalr_o_m:
            offset, reg = jalr_o_m.groups()
            return JumpInstruction(
                caller_addr=addr,
                caller_func=self.current_func,
                insn_type='jalr',
                insn_hex=hexcode,
                register=reg,
                raw_line=raw,
            )

        # jalr reg
        jalr_r_m = self._JALR_REG_RE.match(insn)
        if jalr_r_m:
            reg = jalr_r_m.group(1)
            return JumpInstruction(
                caller_addr=addr,
                caller_func=self.current_func,
                insn_type='jalr',
                insn_hex=hexcode,
                register=reg,
                raw_line=raw,
            )

        return None
