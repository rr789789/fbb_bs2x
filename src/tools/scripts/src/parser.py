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
    - 格式无关: 不强制依赖地址列（行号），适应不同 lst 格式
"""

import re
from typing import List

from .models import JumpInstruction


class JumpParser:
    """跳转指令解析器

    不强制依赖地址列格式，支持以下 lst 行格式:
        9013bd78:\t63a050ef\t\tjal\tra,901413b2 <func>   (有地址列)
        \t63a050ef\t\tjal\tra,901413b2 <func>            (无地址列)
        63a050ef jal ra,901413b2 <func>                    (简化格式)
    """

    # 函数标签行: 9013bd68 <evt_task_gle_scan_parse_rr_ptr>:
    _FUNC_LABEL_RE = re.compile(r'^([0-9a-fA-F]+)\s+<([^>]+)>:$')

    # 函数名行: evt_task_gle_scan_parse_rr_ptr():
    _FUNC_NAME_RE = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*)\(\):$')

    # 地址列剥离: 匹配行首的地址+冒号/点/空格（变为可选）
    _ADDR_STRIP_RE = re.compile(r'^[0-9a-fA-F]+[:.]?\s+')

    # 机器码+指令: 从已剥离地址的行中匹配
    _HEX_INSN_RE = re.compile(
        r'^([0-9a-fA-F]{2,8})\s+(.*)$'
    )

    # 从原始行提取地址（如果有）
    _ADDR_EXTRACT_RE = re.compile(r'^([0-9a-fA-F]+)[:.]')

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

        # 快速筛选：行中必须包含 jal 指令（减少无意义的正则匹配）
        # 使用 tab 或空格包围来避免匹配注释中的 "jal" 子串
        if not self._contains_jump_insn(line):
            return None

        return self._parse_instruction_line(line)

    @staticmethod
    def _contains_jump_insn(line: str) -> bool:
        """检查行中是否包含 jal/jalr 指令（而非注释中的子串）"""
        # 检查 tab 分隔格式: \tjal\t 或 \tjalr\t
        if '\tjal\t' in line or '\tjalr\t' in line:
            return True
        # 检查空格分隔格式:  jal  或  jalr  （前后有空格/tab）
        if ' jal ' in line or ' jalr ' in line:
            return True
        # 行首格式（地址后紧跟 tab/空格）
        if re.search(r':\s+jal\b', line) or re.search(r':\s+jalr\b', line):
            return True
        return False

    def _parse_instruction_line(self, line: str) -> JumpInstruction | None:
        """
        解析指令行。

        步骤:
            1. 剥离可选的地址列（不强制依赖）
            2. 匹配机器码和指令部分
            3. 提取原始地址（如果有）
            4. 解析指令操作数
        """
        # 1. 剥离可选的地址列，得到 "机器码 [whitespace] 指令" 部分
        stripped = self._ADDR_STRIP_RE.sub('', line)

        # 2. 匹配机器码和指令
        m = self._HEX_INSN_RE.match(stripped)
        if not m:
            return None

        hexcode, insn_part = m.groups()

        # 3. 从原始行提取地址（可选）
        addr_match = self._ADDR_EXTRACT_RE.match(line)
        addr = addr_match.group(1) if addr_match else ''

        # 4. 解析具体指令
        return self._parse_instruction(addr, hexcode, insn_part, line)

    def _parse_instruction(self, addr: str, hexcode: str,
                           insn: str, raw: str) -> JumpInstruction | None:
        """解析具体指令操作数，生成 JumpInstruction"""
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
