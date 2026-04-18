#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
跳转指令分析模块

职责:
    对解析后的 JumpInstruction 列表进行统计分析，
    识别高频重复调用模式，输出合并优化建议。

设计原则:
    - 纯计算: 只读输入数据，不修改原始解析结果
    - 可测试: 不依赖文件系统，单元测试友好
    - 可扩展: 分析策略可通过配置参数调整
"""

from typing import List, Dict
from collections import defaultdict, Counter

from .models import (
    JumpInstruction,
    AnalysisResult,
    CallerPattern,
    HighFreqPair,
    MergeCandidate,
)


class JumpAnalyzer:
    """跳转指令分析器"""

    # 合并建议阈值配置
    STRONG_THRESHOLD = 5
    SUGGEST_THRESHOLD = 3

    def __init__(self, strong_threshold: int = 5, suggest_threshold: int = 3):
        self.strong_threshold = strong_threshold
        self.suggest_threshold = suggest_threshold

    def analyze(self, instructions: List[JumpInstruction]) -> AnalysisResult:
        """
        分析跳转指令列表，返回完整的分析结果。

        Args:
            instructions: JumpInstruction 列表

        Returns:
            AnalysisResult 分析结果对象
        """
        by_caller = self._group_by_caller(instructions)

        result = AnalysisResult()

        for caller, items in by_caller.items():
            pattern = self._analyze_single_caller(caller, items)
            result.caller_patterns[caller] = pattern

            # 提取高频对和合并建议
            hf_pairs, candidates = self._extract_patterns(pattern)
            result.high_freq_pairs.extend(hf_pairs)
            result.merge_candidates.extend(candidates)

        # 排序
        result.high_freq_pairs.sort(key=lambda x: (-x.count, x.caller, x.target))
        result.merge_candidates.sort(
            key=lambda x: (-x.potential_saving, x.caller, x.target)
        )
        result.caller_summary = self._build_summary(result.caller_patterns)

        return result

    @staticmethod
    def _group_by_caller(instructions: List[JumpInstruction]) -> Dict[str, List[JumpInstruction]]:
        """按调用者函数分组"""
        groups: Dict[str, List[JumpInstruction]] = defaultdict(list)
        for instr in instructions:
            groups[instr.caller_func].append(instr)
        return groups

    def _analyze_single_caller(self, caller: str,
                               items: List[JumpInstruction]) -> CallerPattern:
        """分析单个调用者函数的跳转模式"""
        total = len(items)
        target_counter = Counter()
        type_map: Dict[str, List[str]] = defaultdict(list)
        internal_count = 0

        for item in items:
            if item.target_func:
                target_counter[item.target_func] += 1
                type_map[item.target_func].append(item.insn_type)
                if item.is_internal_jump():
                    internal_count += 1

        unique = len(target_counter)
        max_repeat = max(target_counter.values()) if target_counter else 0

        return CallerPattern(
            caller_name=caller,
            total_jumps=total,
            unique_targets=unique,
            max_repeat=max_repeat,
            internal_jumps=internal_count,
            target_counter=target_counter,
            type_map=dict(type_map),
        )

    def _extract_patterns(self, pattern: CallerPattern) -> tuple:
        """
        从单个调用者模式中提取高频对和合并建议。

        Returns:
            (high_freq_pairs_list, merge_candidates_list)
        """
        high_freq_pairs: List[HighFreqPair] = []
        merge_candidates: List[MergeCandidate] = []

        for target, count in pattern.target_counter.items():
            if count < 2:
                continue
            # 跳过函数内部跳转
            if self._is_internal_jump(pattern.caller_name, target):
                continue

            types = pattern.type_map.get(target, [])
            type_str = '/'.join(sorted(set(types)))

            high_freq_pairs.append(HighFreqPair(
                caller=pattern.caller_name,
                target=target,
                count=count,
                insn_types=type_str,
            ))

            saving = count - 1
            suggestion = self._build_suggestion(count)

            merge_candidates.append(MergeCandidate(
                caller=pattern.caller_name,
                target=target,
                repeat_count=count,
                potential_saving=saving,
                suggestion=suggestion,
            ))

        return high_freq_pairs, merge_candidates

    @staticmethod
    def _is_internal_jump(caller: str, target: str) -> bool:
        """判断是否为函数内部跳转"""
        if target == caller:
            return True
        if target.startswith(caller + '+'):
            return True
        return False

    def _build_suggestion(self, count: int) -> str:
        """根据重复次数生成优化建议"""
        if count >= self.strong_threshold:
            return ("强烈建议合并：该函数被重复调用 >=5 次，"
                    "可考虑缓存结果或批量处理")
        elif count >= self.suggest_threshold:
            return ("建议合并：重复调用 >=3 次，"
                    "建议检查是否可缓存中间结果")
        else:
            return ("可考虑合并：重复调用 2 次，"
                    "若返回值不变建议缓存")

    @staticmethod
    def _build_summary(patterns: Dict[str, CallerPattern]) -> List:
        """构建调用者摘要列表，按 max_repeat 降序"""
        summary = [
            (p.caller_name, p.total_jumps, p.unique_targets, p.max_repeat)
            for p in patterns.values()
        ]
        summary.sort(key=lambda x: (-x[3], -x[1]))
        return summary
