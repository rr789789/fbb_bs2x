#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
数据模型定义模块

定义解析和分析过程中使用的核心数据结构，
确保模块间数据传递接口清晰、类型安全。
"""

from dataclasses import dataclass, field
from typing import Optional, List, Dict, Tuple
from collections import Counter


@dataclass
class JumpInstruction:
    """单条跳转指令记录"""
    caller_addr: str          # 调用者指令地址
    caller_func: str          # 调用者所在函数名
    insn_type: str            # 'jal' 或 'jalr'
    insn_hex: str             # 指令机器码
    register: str             # 跳转使用的寄存器
    target_addr: Optional[str] = None   # 目标地址（可能未知）
    target_func: Optional[str] = None   # 目标函数名（可能未知）
    raw_line: str = ''        # 原始行内容

    def is_target_resolved(self) -> bool:
        """目标地址是否已解析"""
        return self.target_func is not None

    def is_internal_jump(self) -> bool:
        """是否为函数内部跳转（caller 和 target 属于同一函数）"""
        if not self.target_func:
            return False
        if self.target_func == self.caller_func:
            return True
        if self.target_func.startswith(self.caller_func + '+'):
            return True
        return False


@dataclass
class CallerPattern:
    """单个调用者函数的跳转模式"""
    caller_name: str
    total_jumps: int
    unique_targets: int
    max_repeat: int
    internal_jumps: int
    target_counter: Counter = field(default_factory=Counter)
    type_map: Dict[str, List[str]] = field(default_factory=dict)


@dataclass
class HighFreqPair:
    """高频重复调用对（同一调用者多次调用同一目标）"""
    caller: str
    target: str
    count: int
    insn_types: str         # jal/jalr 类型组合


@dataclass
class MergeCandidate:
    """合并优化候选项"""
    caller: str
    target: str
    repeat_count: int
    potential_saving: int
    suggestion: str


@dataclass
class AnalysisResult:
    """分析结果聚合"""
    caller_patterns: Dict[str, CallerPattern] = field(default_factory=dict)
    high_freq_pairs: List[HighFreqPair] = field(default_factory=list)
    merge_candidates: List[MergeCandidate] = field(default_factory=list)
    caller_summary: List[Tuple[str, int, int, int]] = field(default_factory=list)

    def total_potential_saving(self) -> int:
        """潜在可节省的总调用次数"""
        return sum(m.potential_saving for m in self.merge_candidates)

    def high_freq_count(self) -> int:
        """高频重复调用对数量"""
        return len(self.high_freq_pairs)

    def internal_jumps_count(self) -> int:
        """内部跳转总数"""
        return sum(p.internal_jumps for p in self.caller_patterns.values())
