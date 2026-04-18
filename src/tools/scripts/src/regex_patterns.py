#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
正则表达式配置模块

职责:
    集中管理 lst 文件解析所需的所有正则表达式，
    提供可扩展的配置接口，便于适配不同格式的 lst 文件。

使用方法:
    1. 直接使用默认配置:
       from src.regex_patterns import DEFAULT_CONFIG
       parser = JumpParser(DEFAULT_CONFIG)

    2. 自定义配置（适配新格式）:
       custom = RegexConfig(
           func_label_re=r'^([0-9a-fA-F]+)\s+<([^>]+)>:$',
           ...
       )
       parser = JumpParser(custom)
"""

from dataclasses import dataclass, field
from typing import Callable, List
import re


@dataclass
class RegexConfig:
    """
    正则表达式配置类

    每个字段对应一个匹配规则，使用 Python 原始字符串。
    若某个字段为 None，则该功能被禁用。
    """

    # ------------------------------------------------------------------
    # 函数识别正则
    # ------------------------------------------------------------------

    #: 函数标签行，匹配类似: ``9013bd68 <func_name>:``
    #: Group 1: 地址 (str), Group 2: 函数名 (str)
    func_label_re: str = r'^([0-9a-fA-F]+)\s+<([^>]+)>:$'

    #: 函数名定义行，匹配类似: ``func_name():``
    #: Group 1: 函数名 (str)
    func_name_re: str = r'^([a-zA-Z_][a-zA-Z0-9_]*)\(\):$'

    # ------------------------------------------------------------------
    # 指令行格式正则
    # ------------------------------------------------------------------

    #: 地址列剥离正则（可选匹配）
    #: 匹配行首的地址 + 分隔符（冒号/点/空格），用于移除地址列
    addr_strip_re: str = r'^[0-9a-fA-F]+[:.]?\s+'

    #: 机器码 + 指令匹配正则（剥离地址后使用）
    #: Group 1: 机器码 hex (str, 2~8位), Group 2: 指令文本 (str)
    hex_insn_re: str = r'^([0-9a-fA-F]{2,8})\s+(.*)$'

    #: 从原始行提取地址（可选）
    #: Group 1: 地址 (str)
    addr_extract_re: str = r'^([0-9a-fA-F]+)[:.]'

    # ------------------------------------------------------------------
    # 跳转指令识别正则
    # ------------------------------------------------------------------

    #: jal 指令格式: ``jal ra,901413b2 <func_name>``
    #: Group 1: 寄存器, Group 2: 目标地址, Group 3: 目标函数名 (可选)
    jal_re: str = r'^jal\s+(\w+),\s*([0-9a-fA-F]+)(?:\s+<([^>]+)>)?'

    #: jalr 带目标地址格式: ``jalr offset(reg) # addr <func_name>``
    #: Group 1: 偏移量, Group 2: 寄存器, Group 3: 目标地址, Group 4: 目标函数名 (可选)
    jalr_target_re: str = (
        r'^jalr\s+([-\d]+)\((\w+)\)\s*#\s*([0-9a-fA-F]+)'
        r'(?:\s+<([^>]+)>)?'
    )

    #: jalr 仅寄存器格式: ``jalr reg``
    #: Group 1: 寄存器
    jalr_reg_re: str = r'^jalr\s+(\w+)$'

    #: jalr 偏移+寄存器格式（无目标地址）: ``jalr offset(reg)``
    #: Group 1: 偏移量, Group 2: 寄存器
    jalr_offset_re: str = r'^jalr\s+([-\d]+)\((\w+)\)$'

    # ------------------------------------------------------------------
    # 快速筛选配置
    # ------------------------------------------------------------------

    #: 行中是否包含 jal/jalr 指令的快速判断方法
    #: 返回 True 表示该行可能包含跳转指令，值得进一步解析
    jump_filter: Callable[[str], bool] = field(repr=False, default=None)

    def __post_init__(self):
        if self.jump_filter is None:
            self.jump_filter = self._default_jump_filter

    @staticmethod
    def _default_jump_filter(line: str) -> bool:
        """默认快速筛选：检查行中是否包含 jal/jalr 指令关键字"""
        # tab 分隔格式: \tjal\t 或 \tjalr\t
        if '\tjal\t' in line or '\tjalr\t' in line:
            return True
        # 空格分隔格式:  jal  或  jalr  （前后有空格/tab）
        if ' jal ' in line or ' jalr ' in line:
            return True
        # 行首格式（地址后紧跟 tab/空格）
        if re.search(r':\s+jal\b', line) or re.search(r':\s+jalr\b', line):
            return True
        return False

    def get_compiled_patterns(self) -> dict:
        """
        编译所有正则表达式并返回字典。

        返回的字典键名与字段名一致，值为编译后的 Pattern 对象或 Callable。
        """
        patterns = {}
        for field_name in [
            'func_label_re', 'func_name_re', 'addr_strip_re',
            'hex_insn_re', 'addr_extract_re', 'jal_re',
            'jalr_target_re', 'jalr_reg_re', 'jalr_offset_re',
        ]:
            val = getattr(self, field_name)
            if val is not None:
                patterns[field_name] = re.compile(val)
        patterns['jump_filter'] = self.jump_filter
        return patterns


# ------------------------------------------------------------------------------
# 预设配置实例
# ------------------------------------------------------------------------------

DEFAULT_CONFIG = RegexConfig()
"""默认配置，适配标准 objdump 输出格式"""


# 以下为不同 lst 格式的预设配置模板（使用时取消注释并调整）

NO_ADDR_CONFIG = RegexConfig(
    addr_strip_re=r'^\s+',           # 仅剥离前导空白
    hex_insn_re=r'^([0-9a-fA-F]{2,8})\s+(.*)$',
    addr_extract_re=None,             # 不提取地址
)
"""无地址列格式配置模板"""


DOT_ADDR_CONFIG = RegexConfig(
    addr_strip_re=r'^[0-9a-fA-F]+\.\s+',
    addr_extract_re=r'^([0-9a-fA-F]+)\.',
)
"""地址使用点号分隔的格式配置模板"""


SHORT_HEX_CONFIG = RegexConfig(
    hex_insn_re=r'^([0-9a-fA-F]{2,4})\s+(.*)$',
)
"""机器码较短（16位指令集）的格式配置模板"""
