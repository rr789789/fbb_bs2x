#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LST 文件跳转指令分析工具 - 主入口

功能:
    1. 解析 lst 文件，提取所有 jal/jalr 跳转指令
    2. 分析调用者模式，识别高频重复调用
    3. 生成 txt/json/csv 多格式报告

模块划分:
    src/parser.py   - 负责 lst 文件解析（输入: 文件路径, 输出: 指令列表）
    src/analyzer.py - 负责模式分析（输入: 指令列表, 输出: 分析结果）
    src/reporter.py - 负责报告生成（输入: 指令列表+分析结果, 输出: 报告文件）

用法:
    python3 analyze_jal.py <lst_file>

输出:
    在 lst 文件同目录下创建 ins_output/ 目录，存放:
        - jal_analysis_report.txt      文本分析报告
        - jal_analysis_detail.json     原始指令数据
        - jal_merge_analysis.json      合并分析数据
        - jal_analysis_detail.csv      CSV 表格
"""

import sys
import os

# 将 src 目录加入模块搜索路径
_SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
if _SCRIPT_DIR not in sys.path:
    sys.path.insert(0, _SCRIPT_DIR)

from src import JumpParser, JumpAnalyzer, JumpReporter


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

    # ---- Step 1: 解析 ----
    print(f"[*] 正在解析: {lst_path}")
    parser = JumpParser()
    instructions = parser.parse_file(lst_path)
    print(f"[*] 共发现 {len(instructions)} 条跳转指令")

    # ---- Step 2: 分析 ----
    print("[*] 正在分析调用者模式...")
    analyzer = JumpAnalyzer()
    analysis = analyzer.analyze(instructions)
    print(f"[*] 发现 {analysis.high_freq_count()} 组高频重复调用")

    # ---- Step 3: 生成报告 ----
    reporter = JumpReporter(output_dir, lst_filename)

    report_path = reporter.generate_txt_report(instructions, analysis)
    print(f"[*] 文本报告已保存: {report_path}")

    detail_json = reporter.generate_detail_json(instructions)
    print(f"[*] JSON 数据已保存: {detail_json}")

    merge_json = reporter.generate_merge_json(instructions, analysis)
    print(f"[*] 合并分析 JSON 已保存: {merge_json}")

    csv_path = reporter.generate_csv(instructions)
    print(f"[*] CSV 表格已保存: {csv_path}")

    print("[*] 全部完成!")


if __name__ == '__main__':
    main()
