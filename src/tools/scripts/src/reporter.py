#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
报告生成模块

职责:
    根据解析结果和分析结果，生成多种格式的报告文件。

支持格式:
    - .txt  文本报告（人类可读）
    - .json 详细数据（程序处理）
    - .csv  表格数据（Excel 分析）
    - .json 合并分析（模式分析专用）

设计原则:
    - 与数据源解耦: 不依赖 parser/analyzer 的具体实现
    - 单文件输出: 每个方法生成一个文件
    - 格式独立: 新增格式不修改已有代码
"""

import os
import json
from typing import List
from datetime import datetime

from .models import JumpInstruction, AnalysisResult, CallerPattern


class JumpReporter:
    """跳转指令分析报告生成器"""

    def __init__(self, output_dir: str, lst_filename: str):
        self.output_dir = output_dir
        self.lst_filename = lst_filename
        self.timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

    # ------------------------------------------------------------------
    # 公共辅助方法
    # ------------------------------------------------------------------

    @staticmethod
    def _is_internal_jump(caller: str, target: str) -> bool:
        """判断 target 是否属于 caller 的内部跳转"""
        if target == caller:
            return True
        if target.startswith(caller + '+'):
            return True
        return False

    # ------------------------------------------------------------------
    # TXT 文本报告
    # ------------------------------------------------------------------

    def generate_txt_report(self, instructions: List[JumpInstruction],
                            analysis: AnalysisResult) -> str:
        """生成文本报告，返回输出文件路径"""
        lines = self._build_header()
        lines.extend(self._build_overall_stats(instructions))
        lines.extend(self._build_reg_stats(instructions))
        lines.extend(self._build_top_targets(instructions))
        lines.extend(self._build_top_callers(analysis))
        lines.extend(self._build_freq_analysis(analysis))
        lines.extend(self._build_detail_list(instructions))

        content = '\n'.join(lines)
        path = os.path.join(self.output_dir, 'jal_analysis_report.txt')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)
        return path

    def _build_header(self) -> List[str]:
        """报告头部信息"""
        return [
            '=' * 100,
            '  LST 文件跳转指令分析报告',
            '=' * 100,
            f'  分析文件: {self.lst_filename}',
            f'  分析时间: {self.timestamp}',
            f'  输出目录: {self.output_dir}',
            '=' * 100,
            '',
        ]

    def _build_overall_stats(self, instructions: List[JumpInstruction]) -> List[str]:
        """总体统计"""
        total = len(instructions)
        jal = sum(1 for r in instructions if r.insn_type == 'jal')
        jalr = total - jal
        jalr_resolved = sum(
            1 for r in instructions
            if r.insn_type == 'jalr' and r.target_addr is not None
        )
        return [
            '-' * 100,
            '一、总体统计',
            '-' * 100,
            f'  跳转指令总数:     {total}',
            f'    - jal  指令数:  {jal}',
            f'    - jalr 指令数:  {jalr}',
            f'      * 已解析目标: {jalr_resolved}',
            f'      * 未解析目标: {jalr - jalr_resolved} (间接跳转，目标地址运行时确定)',
            '',
        ]

    def _build_reg_stats(self, instructions: List[JumpInstruction]) -> List[str]:
        """按寄存器统计"""
        from collections import Counter
        reg_counter = Counter(r.register for r in instructions)
        lines = [
            '-' * 100,
            '二、按跳转寄存器统计',
            '-' * 100,
        ]
        for reg, cnt in reg_counter.most_common():
            lines.append(f'  {reg:8s}: {cnt:6d} 条')
        lines.append('')
        return lines

    def _build_top_targets(self, instructions: List[JumpInstruction]) -> List[str]:
        """被调用最频繁的目标函数 TOP 30"""
        from collections import defaultdict
        by_target = defaultdict(list)
        for r in instructions:
            if r.target_func:
                by_target[r.target_func].append(r)
        sorted_targets = sorted(by_target.items(), key=lambda x: -len(x[1]))
        lines = [
            '-' * 100,
            '三、被调用最频繁的目标函数 TOP 30',
            '-' * 100,
        ]
        for idx, (target, items) in enumerate(sorted_targets[:30], 1):
            lines.append(f'  {idx:2d}. {target:50s} 被调用 {len(items):5d} 次')
        lines.append('')
        return lines

    def _build_top_callers(self, analysis: AnalysisResult) -> List[str]:
        """包含跳转指令最多的调用者函数 TOP 30"""
        lines = [
            '-' * 100,
            '四、包含跳转指令最多的调用者函数 TOP 30',
            '-' * 100,
        ]
        for idx, (caller, total, unique, max_r) in enumerate(analysis.caller_summary[:30], 1):
            pattern = analysis.caller_patterns.get(caller)
            if pattern:
                total = pattern.total_jumps
                unique = pattern.unique_targets
                max_r = pattern.max_repeat
            lines.append(
                f'  {idx:2d}. {caller:50s} 共 {total:5d} 条  '
                f'涉及 {unique:4d} 个不同目标  最大重复 {max_r:3d} 次'
            )
        lines.append('')
        return lines

    def _build_freq_analysis(self, analysis: AnalysisResult) -> List[str]:
        """高频重复调用分析"""
        total_hf = analysis.high_freq_count()
        total_saving = analysis.total_potential_saving()
        total_internal = analysis.internal_jumps_count()

        lines = [
            '-' * 100,
            '五、高频重复调用分析（同一调用者多次调用同一目标）',
            '-' * 100,
            f'  发现 {total_hf} 组高频重复调用（跨函数），'
            f'潜在可节省 {total_saving} 次跳转开销',
            f'  （已排除函数内部局部跳转 {total_internal} 条，仅统计跨函数调用）',
            '',
        ]

        # 5.1 高频重复调用对 TOP 50
        lines.append('  5.1 高频重复调用对 TOP 50（跨函数重复调用）')
        lines.append('  ' + '-' * 96)
        lines.append(
            f'  {"序号":>4s}  {"调用者函数":>45s}  '
            f'{"目标函数":>35s}  {"次数":>5s}  {"类型":>8s}'
        )
        lines.append('  ' + '-' * 96)
        for idx, pair in enumerate(analysis.high_freq_pairs[:50], 1):
            lines.append(
                f'  {idx:4d}  {pair.caller:>45s}  '
                f'{pair.target:>35s}  {pair.count:5d}  {pair.insn_types:>8s}'
            )
        lines.append('')

        # 5.2 合并优化建议 TOP 30
        lines.append('  5.2 合并优化建议 TOP 30（按可节省调用次数排序）')
        lines.append('  ' + '-' * 96)
        lines.append(
            f'  {"序号":>4s}  {"调用者函数":>40s}  '
            f'{"目标函数":>30s}  {"重复":>4s}  {"可省":>4s}  {"建议":>40s}'
        )
        lines.append('  ' + '-' * 96)
        for idx, cand in enumerate(analysis.merge_candidates[:30], 1):
            lines.append(
                f'  {idx:4d}  {cand.caller:>40s}  '
                f'{cand.target:>30s}  {cand.repeat_count:4d}  '
                f'{cand.potential_saving:4d}  {cand.suggestion}'
            )
        lines.append('')

        # 5.3 调用者详情 TOP 20
        lines.append('  5.3 各调用者函数的跳转目标分布详情（TOP 20 高频调用者）')
        lines.append('  ' + '-' * 96)
        for idx, (caller, _, _, _) in enumerate(analysis.caller_summary[:20], 1):
            pattern = analysis.caller_patterns.get(caller)
            if not pattern:
                continue
            lines.append(f'  [{idx}] 调用者: {caller}')
            lines.append(
                f'       总跳转数: {pattern.total_jumps}  '
                f'不同目标数: {pattern.unique_targets}  '
                f'最大重复: {pattern.max_repeat}  '
                f'内部跳转: {pattern.internal_jumps}'
            )
            lines.append('       目标分布:')
            for t, c in pattern.target_counter.most_common():
                if self._is_internal_jump(caller, t):
                    mark = "  <-- 函数内部跳转"
                elif c >= 2:
                    mark = "  <-- 可合并"
                else:
                    mark = ""
                lines.append(f'         - {t:45s} : {c:4d} 次{mark}')
            lines.append('')
        lines.append('')
        return lines

    def _build_detail_list(self, instructions: List[JumpInstruction]) -> List[str]:
        """跳转指令详细列表"""
        lines = [
            '-' * 100,
            '六、跳转指令详细列表',
            '-' * 100,
            f'{"序号":>6s}  {"调用者地址":>12s}  {"调用者函数":>40s}  '
            f'{"类型":>5s}  {"寄存器":>8s}  {"目标地址":>12s}  {"目标函数":>40s}',
            '-' * 120,
        ]
        for idx, r in enumerate(instructions, 1):
            caller_addr = f"0x{r.caller_addr}"
            target_addr = f"0x{r.target_addr}" if r.target_addr else 'N/A'
            target_func = r.target_func if r.target_func else 'N/A'
            caller_func = r.caller_func[:38]
            target_func = target_func[:38]
            lines.append(
                f"{idx:6d}  {caller_addr:>12s}  {caller_func:>40s}  "
                f"{r.insn_type:>5s}  {r.register:>8s}  {target_addr:>12s}  "
                f"{target_func:>40s}"
            )
        lines.append('-' * 120)
        lines.append('')
        lines.append('报告生成完毕。')
        return lines

    # ------------------------------------------------------------------
    # JSON 详细数据
    # ------------------------------------------------------------------

    def generate_detail_json(self, instructions: List[JumpInstruction]) -> str:
        """生成原始数据 JSON"""
        data = [
            {
                'caller_addr': r.caller_addr,
                'caller_func': r.caller_func,
                'insn_type': r.insn_type,
                'insn_hex': r.insn_hex,
                'register': r.register,
                'target_addr': r.target_addr,
                'target_func': r.target_func,
                'raw_line': r.raw_line,
            }
            for r in instructions
        ]
        path = os.path.join(self.output_dir, 'jal_analysis_detail.json')
        with open(path, 'w', encoding='utf-8') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
        return path

    # ------------------------------------------------------------------
    # JSON 合并分析
    # ------------------------------------------------------------------

    def generate_merge_json(self, instructions: List[JumpInstruction],
                            analysis: AnalysisResult) -> str:
        """生成合并分析 JSON"""
        report = {
            'summary': {
                'total_jumps': len(instructions),
                'jal_count': sum(1 for r in instructions if r.insn_type == 'jal'),
                'jalr_count': sum(1 for r in instructions if r.insn_type == 'jalr'),
                'high_freq_pairs_count': analysis.high_freq_count(),
                'total_potential_saving': analysis.total_potential_saving(),
                'internal_jumps_count': analysis.internal_jumps_count(),
            },
            'high_freq_pairs': [
                {
                    'caller': p.caller,
                    'target': p.target,
                    'count': p.count,
                    'insn_types': p.insn_types,
                }
                for p in analysis.high_freq_pairs
            ],
            'merge_candidates': [
                {
                    'caller': c.caller,
                    'target': c.target,
                    'repeat_count': c.repeat_count,
                    'potential_saving': c.potential_saving,
                    'suggestion': c.suggestion,
                }
                for c in analysis.merge_candidates
            ],
            'caller_details': {
                name: {
                    'total_jumps': p.total_jumps,
                    'unique_targets': p.unique_targets,
                    'internal_jumps': p.internal_jumps,
                    'target_distribution': dict(p.target_counter.most_common()),
                }
                for name, p in analysis.caller_patterns.items()
            },
        }
        path = os.path.join(self.output_dir, 'jal_merge_analysis.json')
        with open(path, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        return path

    # ------------------------------------------------------------------
    # CSV 表格
    # ------------------------------------------------------------------

    def generate_csv(self, instructions: List[JumpInstruction]) -> str:
        """生成 CSV 表格"""
        path = os.path.join(self.output_dir, 'jal_analysis_detail.csv')
        with open(path, 'w', encoding='utf-8') as f:
            f.write(
                '序号,调用者地址,调用者函数,指令类型,机器码,'
                '寄存器,目标地址,目标函数,原始行\n'
            )
            for idx, r in enumerate(instructions, 1):
                caller_addr = f"0x{r.caller_addr}"
                target_addr = f"0x{r.target_addr}" if r.target_addr else ''
                target_func = r.target_func if r.target_func else ''
                raw = r.raw_line.replace('"', '""')
                f.write(
                    f'{idx},{caller_addr},"{r.caller_func}",'
                    f'{r.insn_type},{r.insn_hex},{r.register},'
                    f'{target_addr},"{target_func}","{raw}"\n'
                )
        return path
