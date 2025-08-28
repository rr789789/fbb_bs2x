#ifndef CHINESE_FONT_H
#define CHINESE_FONT_H

#include <stdint.h>

// 汉字字库结构
typedef struct
{
    const char *utf8_code;    // UTF-8编码的汉字
    const uint8_t *font_data; // 字模数据
} ChineseFont;

// 外部声明
extern const ChineseFont chinese_font_16x16[];
extern const int chinese_font_16x16_count;

// ASCII字库相关
extern const uint8_t ascii_font_8x16[][16];
extern const char ascii_char_map[];
extern const int ascii_font_8x16_count;



#endif // CHINESE_FONT_H