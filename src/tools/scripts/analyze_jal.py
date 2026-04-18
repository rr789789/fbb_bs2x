#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
分析 lst 文件中的跳转指令（jal/jalr），生成调用关系报告。

用法:
    python3 analyze_jal.py <lst_file>
"""

import sys
import os
import re
import json
from collections import defaultdict
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
    
    # 指令行: 9013bd78:	63a050ef          	jal	ra,901413b2 <evt_task_gle_adv_get_pld_data_offset>
    # 注意：冒号后可能有 tab，然后是 hexcode，然后 tab，然后指令
    # 有的短指令 hexcode 只有 4 位
    insn_line_re = re.compile(
        r'^([0-9a-fA-F]+):\s+([0-9a-fA-F]+)\s+(.*)$'
    )
    
    # jal 格式1: jal ra,0x901413b2 <func> 或 jal ra,901413b2 <func>
    # 也可能没有 <func> 部分
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


def generate_report(results, output_dir, lst_filename):
    """生成分析报告"""
    
    # 统计信息
    total = len(results)
    jal_count = sum(1 for r in results if r['insn_type'] == 'jal')
    jalr_count = sum(1 for r in results if r['insn_type'] == 'jalr')
    jalr_resolved = sum(1 for r in results if r['insn_type'] == 'jalr' and r['target_addr'] is not None)
    jalr_unresolved = jalr_count - jalr_resolved
    
    # 按调用者函数统计
    by_caller = defaultdict(list)
    for r in results:
        by_caller[r['caller_func']].append(r)
    
    # 按目标函数统计（仅包含已知目标的）
    by_target = defaultdict(list)
    for r in results:
        if r['target_func']:
            by_target[r['target_func']].append(r)
    
    # 按寄存器统计
    by_reg = defaultdict(list)
    for r in results:
        by_reg[r['register']].append(r)
    
    timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    report_lines = []
    report_lines.append('=' * 80)
    report_lines.append('  LST 文件跳转指令分析报告')
    report_lines.append('=' * 80)
    report_lines.append(f'  分析文件: {lst_filename}')
    report_lines.append(f'  分析时间: {timestamp}')
    report_lines.append(f'  输出目录: {output_dir}')
    report_lines.append('=' * 80)
    report_lines.append('')
    
    # 总体统计
    report_lines.append('-' * 80)
    report_lines.append('一、总体统计')
    report_lines.append('-' * 80)
    report_lines.append(f'  跳转指令总数:     {total}')
    report_lines.append(f'    - jal  指令数:  {jal_count}')
    report_lines.append(f'    - jalr 指令数:  {jalr_count}')
    report_lines.append(f'      * 已解析目标: {jalr_resolved}')
    report_lines.append(f'      * 未解析目标: {jalr_unresolved} (间接跳转，目标地址运行时确定)')
    report_lines.append('')
    
    # 按寄存器统计
    report_lines.append('-' * 80)
    report_lines.append('二、按跳转寄存器统计')
    report_lines.append('-' * 80)
    for reg in sorted(by_reg.keys(), key=lambda x: -len(by_reg[x])):
        report_lines.append(f'  {reg:8s}: {len(by_reg[reg]):6d} 条')
    report_lines.append('')
    
    # 调用最频繁的目标函数 TOP 30
    report_lines.append('-' * 80)
    report_lines.append('三、被调用最频繁的目标函数 TOP 30')
    report_lines.append('-' * 80)
    sorted_targets = sorted(by_target.items(), key=lambda x: -len(x[1]))
    for idx, (target, items) in enumerate(sorted_targets[:30], 1):
        report_lines.append(f'  {idx:2d}. {target:50s} 被调用 {len(items):5d} 次')
    report_lines.append('')
    
    # 包含跳转指令最多的调用者函数 TOP 30
    report_lines.append('-' * 80)
    report_lines.append('四、包含跳转指令最多的调用者函数 TOP 30')
    report_lines.append('-' * 80)
    sorted_callers = sorted(by_caller.items(), key=lambda x: -len(x[1]))
    for idx, (caller, items) in enumerate(sorted_callers[:30], 1):
        jal_n = sum(1 for i in items if i['insn_type'] == 'jal')
        jalr_n = sum(1 for i in items if i['insn_type'] == 'jalr')
        report_lines.append(f'  {idx:2d}. {caller:50s} 共 {len(items):5d} 条 (jal:{jal_n}, jalr:{jalr_n})')
    report_lines.append('')
    
    # 详细列表
    report_lines.append('-' * 80)
    report_lines.append('五、跳转指令详细列表')
    report_lines.append('-' * 80)
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
    
    # 生成文本报告
    report_text = generate_report(results, output_dir, lst_filename)
    report_path = os.path.join(output_dir, 'jal_analysis_report.txt')
    with open(report_path, 'w', encoding='utf-8') as f:
        f.write(report_text)
    print(f"[*] 文本报告已保存: {report_path}")
    
    # 生成 JSON 详细数据
    json_path = os.path.join(output_dir, 'jal_analysis_detail.json')
    with open(json_path, 'w', encoding='utf-8') as f:
        json.dump(results, f, indent=2, ensure_ascii=False)
    print(f"[*] JSON 数据已保存: {json_path}")
    
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
