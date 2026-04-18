#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析 lst 文件中的跳转指令（jal/jalr），生成调用关系报告。

功能：
1. 识别 lst 中所有的 jal/jalr 跳转指令
2. 解析跳转目标地址和调用函数
3. 分析每个调用者函数的内部跳转模式
4. 识别高频重复调用（可合并优化）

用法:
    python3 analyze_jal.py <lst_file>
"""

import sys
import os
import re
import json
from collections import defaultdict, Counter
from datetime import datetime


def parse_lst_file(lst_path):
    """
    解析 lst 文件，提取所有 jal/jalr 指令信息。
    
    返回列表，每项为字典:
    {
        'caller_addr': str,      # 调用者指令地址
        'caller_func': str,      # 调用者所在函数
        'insn_type': str,        # 'jal' 或 'jalr'
        'insn_hex': str,         # 指令机器码
        'target_addr': str,      # 目标地址（可能为 None）
        'target_func': str,      # 目标函数名（可能为 None）
        'register': str,         # 跳转使用的寄存器
        'raw_line': str,         # 原始行
    }
    """
    results = []
    current_func = None
    
    # 函数标签行: 9013bd68 <evt_task_gle_scan_parse_rr_ptr>:
    func_label_re = re.compile(r'^([0-9a-fA-F]+)\s+<([^>]+)>:$')
    
    # 函数名行: evt_task_gle_scan_parse_rr_ptr():
    func_name_re = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*)\(\):$')
    
    # 指令行: 9013bd78:\t63a050ef          \tjal\tra,901413b2 <evt_task_gle_adv_get_pld_data_offset>
    insn_line_re = re.compile(
        r'^([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+(.*)$'
    )
    
    # jal 格式: jal ra,901413b2 <func>
    jal_re = re.compile(
        r'^jal\s+(\w+),\s*([0-9a-fA-F]+)(?:\s+<([^>]+)>)?'
    )
    
    # jalr 格式1: jalr offset(reg) # addr <func>
    jalr_with_target_re = re.compile(
        r'^jalr\s+([-\d]+)\((\w+)\)\s*#\s*([0-9a-fA-F]+)(?:\s+<([^>]+)>)?'
    )
    
    # jalr 格式2: jalr reg  (无偏移，无目标地址信息)
    jalr_reg_only_re = re.compile(
        r'^jalr\s+(\w+)$'
    )
    
    # jalr 格式3: jalr offset(reg)  没有 # 后面的目标地址
    jalr_offset_reg_re = re.compile(
        r'^jalr\s+([-\d]+)\((\w+)\)$'
    )
    
    with open(lst_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            line = line.rstrip('\n\r')
            
            # 尝试匹配函数标签
            m = func_label_re.match(line)
            if m:
                current_func = m.group(2)
                continue
            
            # 尝试匹配函数名行
            m = func_name_re.match(line)
            if m:
                current_func = m.group(1)
                continue
            
            # 尝试匹配指令行
            m = insn_line_re.match(line)
            if not m:
                continue
            
            addr_str, hexcode, insn_part = m.groups()
            
            # 检查是否是 jal 指令
            jal_m = jal_re.match(insn_part)
            if jal_m:
                reg, target_addr, target_func = jal_m.groups()
                results.append({
                    'caller_addr': addr_str,
                    'caller_func': current_func or 'UNKNOWN',
                    'insn_type': 'jal',
                    'insn_hex': hexcode,
                    'target_addr': target_addr,
                    'target_func': target_func,
                    'register': reg,
                    'raw_line': line,
                })
                continue
            
            # 检查是否是 jalr 带目标地址
            jalr_t_m = jalr_with_target_re.match(insn_part)
            if jalr_t_m:
                offset, reg, target_addr, target_func = jalr_t_m.groups()
                results.append({
                    'caller_addr': addr_str,
                    'caller_func': current_func or 'UNKNOWN',
                    'insn_type': 'jalr',
                    'insn_hex': hexcode,
                    'target_addr': target_addr,
                    'target_func': target_func,
                    'register': reg,
                    'raw_line': line,
                })
                continue
            
            # 检查 jalr offset(reg) 无目标
            jalr_o_m = jalr_offset_reg_re.match(insn_part)
            if jalr_o_m:
                offset, reg = jalr_o_m.groups()
                results.append({
                    'caller_addr': addr_str,
                    'caller_func': current_func or 'UNKNOWN',
                    'insn_type': 'jalr',
                    'insn_hex': hexcode,
                    'target_addr': None,
                    'target_func': None,
                    'register': reg,
                    'raw_line': line,
                })
                continue
            
            # 检查 jalr reg
            jalr_r_m = jalr_reg_only_re.match(insn_part)
            if jalr_r_m:
                reg = jalr_r_m.group(1)
                results.append({
                    'caller_addr': addr_str,
                    'caller_func': current_func or 'UNKNOWN',
                    'insn_type': 'jalr',
                    'insn_hex': hexcode,
                    'target_addr': None,
                    'target_func': None,
                    'register': reg,
                    'raw_line': line,
                })
                continue
    
    return results


def is_internal_jump(caller, target):
    """判断 target 是否是 caller 函数内部的局部跳转"""
    if target == caller:
        return True
    # target 形如 "func_name+0x123"，检查前缀是否为 caller
    if target.startswith(caller + '+'):
        return True
    return False


def analyze_caller_patterns(results):
    """
    分析每个调用者函数的跳转模式。
    
    返回:
    {
        'caller_stats': {caller_func: Counter(target_func: count)},
        'high_freq_pairs': [(caller, target, count, insn_types)],  # 高频重复调用对（排除内部跳转）
        'merge_candidates': [(caller, target, count, saving, reason)],  # 合并优化候选
        'caller_summary': [(caller, total_jumps, unique_targets, max_repeat)],
        'internal_jumps': {caller: count},  # 每个调用者的内部跳转数
    }
    """
    # 按调用者分组
    by_caller = defaultdict(list)
    for r in results:
        by_caller[r['caller_func']].append(r)
    
    caller_stats = {}
    high_freq_pairs = []       # (caller, target, count, insn_type_str)
    merge_candidates = []      # (caller, target, count, saving, reason)
    caller_summary = []        # (caller, total, unique, max_repeat)
    internal_jumps = {}        # caller -> count
    
    for caller, items in by_caller.items():
        total = len(items)
        # 只统计有明确目标函数的跳转
        target_counter = Counter()
        type_map = defaultdict(list)   # target -> [insn_types]
        internal_count = 0
        
        for item in items:
            if item['target_func']:
                key = item['target_func']
                target_counter[key] += 1
                type_map[key].append(item['insn_type'])
                if is_internal_jump(caller, key):
                    internal_count += 1
        
        caller_stats[caller] = target_counter
        internal_jumps[caller] = internal_count
        
        unique_targets = len(target_counter)
        max_repeat = max(target_counter.values()) if target_counter else 0
        caller_summary.append((caller, total, unique_targets, max_repeat))
        
        # 分析高频重复调用（同一 caller 多次调用同一 target），排除函数内部跳转
        for target, count in target_counter.items():
            if count >= 2 and not is_internal_jump(caller, target):
                types = type_map[target]
                type_str = '/'.join(sorted(set(types)))
                high_freq_pairs.append((caller, target, count, type_str))
                
                # 合并优化建议
                saving = count - 1  # 合并后至少节省 count-1 次调用开销
                if count >= 5:
                    reason = "强烈建议合并：该函数被重复调用 >=5 次，可考虑缓存结果或批量处理"
                elif count >= 3:
                    reason = "建议合并：重复调用 >=3 次，建议检查是否可缓存中间结果"
                else:
                    reason = "可考虑合并：重复调用 2 次，若返回值不变建议缓存"
                
                merge_candidates.append((caller, target, count, saving, reason))
    
    # 排序
    high_freq_pairs.sort(key=lambda x: (-x[2], x[0], x[1]))
    merge_candidates.sort(key=lambda x: (-x[3], x[0], x[1]))
    caller_summary.sort(key=lambda x: (-x[3], -x[1]))
    
    return {
        'caller_stats': caller_stats,
        'high_freq_pairs': high_freq_pairs,
        'merge_candidates': merge_candidates,
        'caller_summary': caller_summary,
        'internal_jumps': internal_jumps,
    }


def generate_report(results, pattern_analysis, output_dir, lst_filename):
    """生成分析报告"""
    
    # 统计信息
    total = len(results)
    jal_count = sum(1 for r in results if r['insn_type'] == 'jal')
    jalr_count = sum(1 for r in results if r['insn_type'] == 'jalr')
    jalr_resolved = sum(1 for r in results if r['insn_type'] == 'jalr' and r['target_addr'] is not None)
    jalr_unresolved = jalr_count - jalr_resolved
    
    # 按目标函数统计（仅包含已知目标的）
    by_target = defaultdict(list)
    for r in results:
        if r['target_func']:
            by_target[r['target_func']].append(r)
    
    # 按寄存器统计
    by_reg = defaultdict(list)
    for r in results:
        by_reg[r['register']].append(r)
    
    # 展开模式分析结果
    caller_stats = pattern_analysis['caller_stats']
    high_freq_pairs = pattern_analysis['high_freq_pairs']
    merge_candidates = pattern_analysis['merge_candidates']
    caller_summary = pattern_analysis['caller_summary']
    internal_jumps = pattern_analysis['internal_jumps']
    
    # 计算可优化总数
    total_saving = sum(m[3] for m in merge_candidates)
    total_high_freq = len(high_freq_pairs)
    total_internal = sum(internal_jumps.values())
    
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    report_lines = []
    report_lines.append('=' * 100)
    report_lines.append('  LST 文件跳转指令分析报告')
    report_lines.append('=' * 100)
    report_lines.append(f'  分析文件: {lst_filename}')
    report_lines.append(f'  分析时间: {timestamp}')
    report_lines.append(f'  输出目录: {output_dir}')
    report_lines.append('=' * 100)
    report_lines.append('')
    
    # 总体统计
    report_lines.append('-' * 100)
    report_lines.append('一、总体统计')
    report_lines.append('-' * 100)
    report_lines.append(f'  跳转指令总数:     {total}')
    report_lines.append(f'    - jal  指令数:  {jal_count}')
    report_lines.append(f'    - jalr 指令数:  {jalr_count}')
    report_lines.append(f'      * 已解析目标: {jalr_resolved}')
    report_lines.append(f'      * 未解析目标: {jalr_unresolved} (间接跳转，目标地址运行时确定)')
    report_lines.append('')
    
    # 按寄存器统计
    report_lines.append('-' * 100)
    report_lines.append('二、按跳转寄存器统计')
    report_lines.append('-' * 100)
    for reg in sorted(by_reg.keys(), key=lambda x: -len(by_reg[x])):
        report_lines.append(f'  {reg:8s}: {len(by_reg[reg]):6d} 条')
    report_lines.append('')
    
    # 调用最频繁的目标函数 TOP 30
    report_lines.append('-' * 100)
    report_lines.append('三、被调用最频繁的目标函数 TOP 30')
    report_lines.append('-' * 100)
    sorted_targets = sorted(by_target.items(), key=lambda x: -len(x[1]))
    for idx, (target, items) in enumerate(sorted_targets[:30], 1):
        report_lines.append(f'  {idx:2d}. {target:50s} 被调用 {len(items):5d} 次')
    report_lines.append('')
    
    # 包含跳转指令最多的调用者函数 TOP 30
    report_lines.append('-' * 100)
    report_lines.append('四、包含跳转指令最多的调用者函数 TOP 30')
    report_lines.append('-' * 100)
    sorted_callers = sorted(caller_stats.items(), key=lambda x: -len(x[1]))
    for idx, (caller, counter) in enumerate(sorted_callers[:30], 1):
        total_jumps = sum(counter.values())
        unique = len(counter)
        max_r = max(counter.values()) if counter else 0
        report_lines.append(
            f'  {idx:2d}. {caller:50s} 共 {total_jumps:5d} 条  涉及 {unique:4d} 个不同目标  最大重复 {max_r:3d} 次'
        )
    report_lines.append('')
    
    # ===== 新增：高频相关调用分析 =====
    report_lines.append('-' * 100)
    report_lines.append('五、高频重复调用分析（同一调用者多次调用同一目标）')
    report_lines.append('-' * 100)
    report_lines.append(f'  发现 {total_high_freq} 组高频重复调用（跨函数），潜在可节省 {total_saving} 次跳转开销')
    report_lines.append(f'  （已排除函数内部局部跳转 {total_internal} 条，仅统计跨函数调用）')
    report_lines.append('')
    
    # 5.1 全局高频重复调用对 TOP 50
    report_lines.append('  5.1 高频重复调用对 TOP 50（跨函数重复调用）')
    report_lines.append('  ' + '-' * 96)
    report_lines.append(f'  {"序号":>4s}  {"调用者函数":>45s}  {"目标函数":>35s}  {"次数":>5s}  {"类型":>8s}')
    report_lines.append('  ' + '-' * 96)
    for idx, (caller, target, count, type_str) in enumerate(high_freq_pairs[:50], 1):
        report_lines.append(
            f'  {idx:4d}  {caller:>45s}  {target:>35s}  {count:5d}  {type_str:>8s}'
        )
    report_lines.append('')
    
    # 5.2 合并优化建议 TOP 30
    report_lines.append('  5.2 合并优化建议 TOP 30（按可节省调用次数排序）')
    report_lines.append('  ' + '-' * 96)
    report_lines.append(f'  {"序号":>4s}  {"调用者函数":>40s}  {"目标函数":>30s}  {"重复":>4s}  {"可省":>4s}  {"建议":>40s}')
    report_lines.append('  ' + '-' * 96)
    for idx, (caller, target, count, saving, reason) in enumerate(merge_candidates[:30], 1):
        report_lines.append(
            f'  {idx:4d}  {caller:>40s}  {target:>30s}  {count:4d}  {saving:4d}  {reason}'
        )
    report_lines.append('')
    
    # 5.3 每个调用者函数的调用分布详情（TOP 20 调用者）
    report_lines.append('  5.3 各调用者函数的跳转目标分布详情（TOP 20 高频调用者）')
    report_lines.append('  ' + '-' * 96)
    for idx, (caller, total_j, unique, max_r) in enumerate(caller_summary[:20], 1):
        counter = caller_stats[caller]
        internal_c = internal_jumps.get(caller, 0)
        report_lines.append(f'  [{idx}] 调用者: {caller}')
        report_lines.append(f'       总跳转数: {total_j}  不同目标数: {unique}  最大重复: {max_r}  内部跳转: {internal_c}')
        report_lines.append(f'       目标分布:')
        # 按调用次数排序
        sorted_targets = counter.most_common()
        for t, c in sorted_targets:
            if is_internal_jump(caller, t):
                mark = "  <-- 函数内部跳转"
            elif c >= 2:
                mark = "  <-- 可合并"
            else:
                mark = ""
            report_lines.append(f'         - {t:45s} : {c:4d} 次{mark}')
        report_lines.append('')
    report_lines.append('')
    
    # 详细列表
    report_lines.append('-' * 100)
    report_lines.append('六、跳转指令详细列表')
    report_lines.append('-' * 100)
    report_lines.append(f'{"序号":>6s}  {"调用者地址":>12s}  {"调用者函数":>40s}  {"类型":>5s}  {"寄存器":>8s}  {"目标地址":>12s}  {"目标函数":>40s}')
    report_lines.append('-' * 120)
    for idx, r in enumerate(results, 1):
        caller_addr = f"0x{r['caller_addr']}"
        target_addr = f"0x{r['target_addr']}" if r['target_addr'] else 'N/A'
        target_func = r['target_func'] if r['target_func'] else 'N/A'
        caller_func = r['caller_func'][:38]
        target_func = target_func[:38]
        report_lines.append(
            f"{idx:6d}  {caller_addr:>12s}  {caller_func:>40s}  {r['insn_type']:>5s}  {r['register']:>8s}  {target_addr:>12s}  {target_func:>40s}"
        )
    report_lines.append('-' * 120)
    report_lines.append('')
    report_lines.append('报告生成完毕。')
    
    return '\n'.join(report_lines)


def generate_json_report(results, pattern_analysis, output_dir):
    """生成 JSON 格式的模式分析报告"""
    json_report = {
        'summary': {
            'total_jumps': len(results),
            'jal_count': sum(1 for r in results if r['insn_type'] == 'jal'),
            'jalr_count': sum(1 for r in results if r['insn_type'] == 'jalr'),
            'high_freq_pairs_count': len(pattern_analysis['high_freq_pairs']),
            'total_potential_saving': sum(m[3] for m in pattern_analysis['merge_candidates']),
            'internal_jumps_count': sum(pattern_analysis['internal_jumps'].values()),
        },
        'high_freq_pairs': [
            {
                'caller': c,
                'target': t,
                'count': n,
                'insn_types': types,
            }
            for c, t, n, types in pattern_analysis['high_freq_pairs']
        ],
        'merge_candidates': [
            {
                'caller': c,
                'target': t,
                'repeat_count': n,
                'potential_saving': s,
                'suggestion': r,
            }
            for c, t, n, s, r in pattern_analysis['merge_candidates']
        ],
        'caller_details': {
            caller: {
                'total_jumps': sum(counter.values()),
                'unique_targets': len(counter),
                'internal_jumps': pattern_analysis['internal_jumps'].get(caller, 0),
                'target_distribution': dict(counter.most_common()),
            }
            for caller, counter in pattern_analysis['caller_stats'].items()
        },
    }
    
    json_path = os.path.join(output_dir, 'jal_merge_analysis.json')
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(json_report, f, indent=2, ensure_ascii=False)
    return json_path


def main():
    if len(sys.argv) < 2:
        print(f"用法: {sys.argv[0]} <lst_file>")
        sys.exit(1)
    
    lst_path = sys.argv[1]
    if not os.path.isfile(lst_path):
        print(f"错误: 文件不存在: {lst_path}")
        sys.exit(1)
    
    lst_path = os.path.abspath(lst_path)
    lst_dir = os.path.dirname(lst_path)
    lst_filename = os.path.basename(lst_path)
    
    # 创建输出目录
    output_dir = os.path.join(lst_dir, 'ins_output')
    os.makedirs(output_dir, exist_ok=True)
    
    print(f"[*] 正在解析: {lst_path}")
    results = parse_lst_file(lst_path)
    print(f"[*] 共发现 {len(results)} 条跳转指令")
    
    print(f"[*] 正在分析调用者模式...")
    pattern_analysis = analyze_caller_patterns(results)
    print(f"[*] 发现 {len(pattern_analysis['high_freq_pairs'])} 组高频重复调用")
    
    # 生成文本报告
    report_text = generate_report(results, pattern_analysis, output_dir, lst_filename)
    report_path = os.path.join(output_dir, 'jal_analysis_report.txt')
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report_text)
    print(f"[*] 文本报告已保存: {report_path}")
    
    # 生成 JSON 详细数据
    json_path = os.path.join(output_dir, 'jal_analysis_detail.json')
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    print(f"[*] JSON 数据已保存: {json_path}")
    
    # 生成 JSON 模式分析报告
    merge_json_path = generate_json_report(results, pattern_analysis, output_dir)
    print(f"[*] 合并分析 JSON 已保存: {merge_json_path}")
    
    # 生成 CSV 表格
    csv_path = os.path.join(output_dir, 'jal_analysis_detail.csv')
    with open(csv_path, 'w', encoding='utf-8') as f:
        f.write('序号,调用者地址,调用者函数,指令类型,机器码,寄存器,目标地址,目标函数,原始行\n')
        for idx, r in enumerate(results, 1):
            caller_addr = f"0x{r['caller_addr']}"
            target_addr = f"0x{r['target_addr']}" if r['target_addr'] else ''
            target_func = r['target_func'] if r['target_func'] else ''
            raw = r['raw_line'].replace('"', '""')
            f.write(f'{idx},{caller_addr},"{r["caller_func"]}",{r["insn_type"]},{r["insn_hex"]},{r["register"]},{target_addr},"{target_func}","{raw}"\n')
    print(f"[*] CSV 表格已保存: {csv_path}")
    
    print("[*] 全部完成!")


if __name__ == '__main__':
    main()
