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
    - 可配置: 正则表达式通过 RegexConfig 注入，支持不同 lst 格式
"""

from typing import List

from .models import JumpInstruction
from .regex_patterns import RegexConfig, DEFAULT_CONFIG


class JumpParser:
    """跳转指令解析器

    支持通过 RegexConfig 注入正则规则，适配不同 lst 格式。
    默认使用标准 objdump 输出格式。
    """

    def __init__(self, regex_config: RegexConfig | None = None):
        """
        初始化解析器。

        Args:
            regex_config: 正则配置对象，None 时使用默认配置
        """
        self.current_func: str = 'UNKNOWN'
        self.cfg = regex_config or DEFAULT_CONFIG
        self._patterns = self.cfg.get_compiled_patterns()

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
        # 1. 尝试匹配函数标签
        pat = self._patterns.get('func_label_re')
        if pat:
            m = pat.match(line)
            if m:
                self.current_func = m.group(2)
                return None

        # 2. 尝试匹配函数名行
        pat = self._patterns.get('func_name_re')
        if pat:
            m = pat.match(line)
            if m:
                self.current_func = m.group(1)
                return None

        # 3. 快速筛选：行中是否包含 jal 指令
        jump_filter = self._patterns.get('jump_filter')
        if jump_filter and not jump_filter(line):
            return None

        return self._parse_instruction_line(line)

    def _parse_instruction_line(self, line: str) -> JumpInstruction | None:
        """
        解析指令行。

        步骤:
            1. 剥离可选的地址列（不强制依赖）
            2. 匹配机器码和指令部分
            3. 提取原始地址（如果有）
            4. 解析指令操作数
        """
        # 1. 剥离可选的地址列
        addr_strip_pat = self._patterns.get('addr_strip_re')
        if addr_strip_pat:
            stripped = addr_strip_pat.sub('', line)
        else:
            stripped = line.lstrip()

        # 2. 匹配机器码和指令
        hex_insn_pat = self._patterns.get('hex_insn_re')
        if not hex_insn_pat:
            return None

        m = hex_insn_pat.match(stripped)
        if not m:
            return None

        hexcode, insn_part = m.groups()

        # 3. 从原始行提取地址（可选）
        addr = ''
        addr_extract_pat = self._patterns.get('addr_extract_re')
        if addr_extract_pat:
            addr_match = addr_extract_pat.match(line)
            if addr_match:
                addr = addr_match.group(1)

        # 4. 解析具体指令
        return self._parse_instruction(addr, hexcode, insn_part, line)

    def _parse_instruction(self, addr: str, hexcode: str,
                           insn: str, raw: str) -> JumpInstruction | None:
        """解析具体指令操作数，生成 JumpInstruction"""
        # jal
        jal_pat = self._patterns.get('jal_re')
        if jal_pat:
            jal_m = jal_pat.match(insn)
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
        jalr_t_pat = self._patterns.get('jalr_target_re')
        if jalr_t_pat:
            jalr_t_m = jalr_t_pat.match(insn)
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
        jalr_o_pat = self._patterns.get('jalr_offset_re')
        if jalr_o_pat:
            jalr_o_m = jalr_o_pat.match(insn)
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
        jalr_r_pat = self._patterns.get('jalr_reg_re')
        if jalr_r_pat:
            jalr_r_m = jalr_r_pat.match(insn)
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
