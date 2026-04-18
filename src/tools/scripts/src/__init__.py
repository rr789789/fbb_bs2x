#!/usr/bin/env python3
# -*- coding: utf-8 -*-
r"""
LST 跳转指令分析工具包

功能模块:
    parser         - 解析 lst 文件，提取跳转指令
    analyzer       - 分析调用者模式，识别高频重复调用
    reporter       - 生成多格式分析报告
    regex_patterns - 正则表达式配置管理

用法示例:
    from src import JumpParser, JumpAnalyzer, JumpReporter, RegexConfig, DEFAULT_CONFIG

    # 使用默认配置
    parser = JumpParser()

    # 使用自定义配置
    custom_cfg = RegexConfig(addr_strip_re=r'^\s+', addr_extract_re=None)
    parser = JumpParser(custom_cfg)
"""

from .regex_patterns import RegexConfig, DEFAULT_CONFIG
from .parser import JumpParser
from .analyzer import JumpAnalyzer
from .reporter import JumpReporter

__all__ = [
    'RegexConfig',
    'DEFAULT_CONFIG',
    'JumpParser',
    'JumpAnalyzer',
    'JumpReporter',
]
