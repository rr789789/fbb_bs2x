# LST 跳转指令分析工具 - 格式适配说明

本文档说明如何为不同格式的 lst 文件适配解析器。

---

## 1. 快速开始

默认配置 `DEFAULT_CONFIG` 适配标准 GNU objdump 输出格式：

```python
from src import JumpParser

parser = JumpParser()  # 使用默认配置
results = parser.parse_file("application.lst")
```

若 lst 格式不同（无地址列、分隔符不同等），只需创建一个 `RegexConfig` 并传入即可。

---

## 2. RegexConfig 字段说明

`RegexConfig` 包含以下字段，每个字段是一个 Python 正则表达式字符串：

| 字段名 | 说明 | 默认正则 | 捕获组 |
|--------|------|----------|--------|
| `func_label_re` | 函数标签行（如 `9013bd68 <func>:`） | `^([0-9a-fA-F]+)\s+<([^>]+)>:$` | G1=地址, G2=函数名 |
| `func_name_re` | 函数名定义行（如 `func():`） | `^([a-zA-Z_][a-zA-Z0-9_]*)\(\):$` | G1=函数名 |
| `addr_strip_re` | 剥离行首地址列 | `^[0-9a-fA-F]+[:.]?\s+` | — |
| `hex_insn_re` | 匹配机器码和指令 | `^([0-9a-fA-F]{2,8})\s+(.*)$` | G1=hex, G2=insn |
| `addr_extract_re` | 从原始行提取地址（可选） | `^([0-9a-fA-F]+)[:.]` | G1=地址 |
| `jal_re` | jal 指令格式 | `^jal\s+(\w+),\s*([0-9a-fA-F]+)...` | G1=reg, G2=addr, G3=func(可选) |
| `jalr_target_re` | jalr 带目标格式 | `^jalr\s+([-\d]+)\((\w+)\)\s*#...` | G1=offset, G2=reg, G3=addr, G4=func(可选) |
| `jalr_reg_re` | jalr 仅寄存器格式 | `^jalr\s+(\w+)$` | G1=reg |
| `jalr_offset_re` | jalr 偏移+寄存器（无目标） | `^jalr\s+([-\d]+)\((\w+)\)$` | G1=offset, G2=reg |
| `jump_filter` | 快速筛选回调函数 | 默认实现 | — |

**将字段设为 `None` 可禁用对应功能。**

---

## 3. 常见格式适配示例

### 3.1 无地址列的 lst 格式

原始行格式：
```
\t63a050ef\t\tjal\tra,901413b2 <func>
```

适配配置：
```python
from src import RegexConfig, JumpParser

cfg = RegexConfig(
    addr_strip_re=r'^\s+',           # 仅剥离前导空白
    addr_extract_re=None,             # 不提取地址
)
parser = JumpParser(cfg)
```

### 3.2 地址使用点号分隔

原始行格式：
```
9013bd78.\t63a050ef\t\tjal\tra,901413b2 <func>
```

适配配置：
```python
cfg = RegexConfig(
    addr_strip_re=r'^[0-9a-fA-F]+\.\s+',
    addr_extract_re=r'^([0-9a-fA-F]+)\.',
)
parser = JumpParser(cfg)
```

### 3.3 短机器码格式（16位指令集）

原始行格式：
```
9013bd78:\t8128\t\tpush\t{ra,s0},-32
```

适配配置：
```python
cfg = RegexConfig(
    hex_insn_re=r'^([0-9a-fA-F]{2,4})\s+(.*)$',
)
parser = JumpParser(cfg)
```

### 3.4 自定义函数标签格式

原始行格式：
```
[func_name] @ 0x9013bd68
```

适配配置：
```python
cfg = RegexConfig(
    func_label_re=r'^\[([^\]]+)\]\s+@\s+(0x[0-9a-fA-F]+)$',
)
# 注意：此格式下 group 顺序变为 G1=函数名, G2=地址
# 需要同步修改 parser.py 中 func_label_re 的 group 索引
```

> **提示**：若函数标签的捕获组顺序改变，需要修改 `parser.py` 中 `_parse_line()` 方法的 group 引用。

### 3.5 完全自定义格式（覆盖所有字段）

```python
from src import RegexConfig, JumpParser

cfg = RegexConfig(
    func_label_re=r'^FUNC\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+@\s+([0-9a-fA-F]+)$',
    func_name_re=None,                # 无函数名定义行
    addr_strip_re=r'^\s*',            # 仅剥离前导空白
    hex_insn_re=r'^([0-9a-fA-F]{8})\s+(.*)$',
    addr_extract_re=None,
    jal_re=r'^JMP\s+(\w+),\s*([0-9a-fA-F]+)(?:\s+\[([^\]]+)\])?',
    jalr_target_re=None,              # 无 jalr 带目标格式
    jalr_reg_re=r'^JMPR\s+(\w+)$',
    jalr_offset_re=None,
    jump_filter=lambda line: 'JMP' in line or 'JMPR' in line,
)

parser = JumpParser(cfg)
```

---

## 4. jump_filter 快速筛选

`jump_filter` 是一个回调函数，用于快速判断某行是否值得进一步正则解析。

**为什么要用它？**

lst 文件通常包含数万行，但只有一小部分是跳转指令。`jump_filter` 先进行 O(1) 的子串检查，避免对每一行都执行复杂的正则匹配，大幅提升解析速度。

**默认实现**：
```python
def _default_jump_filter(line: str) -> bool:
    if '\tjal\t' in line or '\tjalr\t' in line:
        return True
    if ' jal ' in line or ' jalr ' in line:
        return True
    if re.search(r':\s+jal\b', line) or re.search(r':\s+jalr\b', line):
        return True
    return False
```

**自定义示例**（若指令关键字不同）：
```python
cfg = RegexConfig(
    jump_filter=lambda line: 'call' in line.lower(),
)
```

---

## 5. 调试适配过程

### 5.1 单步测试正则

在 Python 交互式环境中测试：
```python
import re

# 测试 addr_strip_re
line = "9013bd78:\t63a050ef\t\tjal\tra,901413b2 <func>"
addr_strip_re = r'^[0-9a-fA-F]+[:.]?\s+'
stripped = re.sub(addr_strip_re, '', line)
print(stripped)  # 应输出: 63a050ef\t\tjal\tra,901413b2 <func>

# 测试 hex_insn_re
hex_insn_re = r'^([0-9a-fA-F]{2,8})\s+(.*)$'
m = re.match(hex_insn_re, stripped)
print(m.groups())  # 应输出: ('63a050ef', 'jal\tra,901413b2 <func>')
```

### 5.2 验证解析结果

```python
from src import JumpParser, RegexConfig

cfg = RegexConfig(...)  # 你的自定义配置
parser = JumpParser(cfg)
results = parser.parse_file("your.lst")

print(f"解析到 {len(results)} 条跳转指令")
for r in results[:5]:
    print(f"  {r.caller_func} -> {r.target_func} ({r.insn_type})")
```

### 5.3 对比预期

1. **数量检查**：解析出的指令数应与手动 grep 统计的 jal/jalr 数一致
2. **地址检查**：检查 `caller_addr` 和 `target_addr` 是否正确提取
3. **函数名检查**：确认 `caller_func` 和 `target_func` 无乱码或截断

---

## 6. 预设配置模板

源码中已提供以下预设配置模板（位于 `src/regex_patterns.py`）：

| 配置名 | 适用场景 |
|--------|----------|
| `DEFAULT_CONFIG` | 标准 objdump 格式（有地址列，冒号分隔） |
| `NO_ADDR_CONFIG` | 无地址列格式 |
| `DOT_ADDR_CONFIG` | 地址使用点号分隔 |
| `SHORT_HEX_CONFIG` | 16位短机器码（如 RISC-V compressed） |

使用方式：
```python
from src.regex_patterns import NO_ADDR_CONFIG
from src import JumpParser

parser = JumpParser(NO_ADDR_CONFIG)
```

---

## 7. 注意事项

1. **正则优先级**：jal > jalr_target > jalr_offset > jalr_reg，按此顺序匹配
2. **None 字段**：设为 `None` 的正则不会参与匹配，对应功能被禁用
3. **捕获组顺序**：修改正则时确保捕获组顺序与解析逻辑一致
4. **性能**：`jump_filter` 的返回值直接影响解析速度，应尽可能简单
5. **编码**：lst 文件可能有 UTF-8 编码问题，parser 已设置 `errors='ignore'`
