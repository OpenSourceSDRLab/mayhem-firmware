/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * copyleft Mr. Robot
 *
 * This file is part of PortaPack.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "ui_painter.hpp"

#include "ui_widget.hpp"

#include "portapack.hpp"
using namespace portapack;

namespace ui {

Style Style::invert() const {
    return {
        .font = font,
        .background = foreground,
        .foreground = background};
}


static void convert_8x16_to_12x16(const uint8_t* src, uint16_t* dst) {
    const uint8_t repeat_map[8] = {2, 2, 1, 2, 1, 1, 2, 1};  // 水平放大规则：总共扩展为12位

    for (int y = 0; y < 16; ++y) {
        uint8_t row = src[y];
        uint16_t expanded_row = 0;
        int bit_pos = 11;  // 从高位向低位填充

        for (int bit = 0; bit < 8; ++bit) {
            uint8_t src_bit = (row >> (7 - bit)) & 1;
            for (int r = 0; r < repeat_map[bit]; ++r) {
                if (src_bit && bit_pos >= 0) {
                    expanded_row |= (1 << bit_pos);
                }
                --bit_pos;
            }
        }

        dst[y] = expanded_row;  // 每行只写一次（共16行）
    }
}



static void convert_8x16_to_12x24(const uint8_t* src, uint16_t* dst) {
    const uint8_t h_repeat[8] = {2, 2, 1, 2, 1, 1, 2, 1}; // 8列变12列
    const uint8_t v_repeat[16] = {
        2, 2, 2, 2, 1, 1, 2, 1,
        2, 1, 1, 1, 1, 1, 2, 2  // 保证视觉均匀，底部不空洞
    };

    int dst_row_index = 0;

    for (int y = 0; y < 16; ++y) {
        uint8_t row = src[y];
        uint16_t expanded_row = 0;
        int bit_pos = 11; // 左边高位

        // 水平方向扩展：构造 12-bit 行
        for (int bit = 0; bit < 8; ++bit) {
            uint8_t pixel = (row >> (7 - bit)) & 1;
            for (int r = 0; r < h_repeat[bit]; ++r) {
                if (pixel && bit_pos >= 0) {
                    expanded_row |= (1 << bit_pos);
                }
                --bit_pos;
            }
        }

        // 垂直方向扩展：将此行重复 v_repeat[y] 次
        for (int vr = 0; vr < v_repeat[y]; ++vr) {
            dst[dst_row_index++] = expanded_row;
        }
    }
}


// 目前看起来8*16变为 8*24好看一点
int Painter::draw_char(Point p, const Style& style, char c, uint8_t zoom_level) {
    if( c >= 0x20 && c<=0x7E)
    {
        uint8_t idx = c-0x20;
        // uint16_t test_char[]=
        // {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x079E, 0x010C, 0x0108, 0x0118, 0x0090, 0x0090, 0x00B0, 0x0060, 0x0060, 0x0020, 0x0020, 0x0020, 0x0014, 0x000C}
        // ;
        // display.draw_glyph_v2(p, ui::Size(12,24), style.foreground, style.background,(void *)test_char);
        display.draw_glyph_v2(p, ui::Size(12,24), style.foreground, style.background,(void *)font12x24[idx]);
        return p.x()* zoom_level;
    }
    else
    {
        const auto glyph = style.font.glyph(c);
        uint16_t output[32];
        convert_8x16_to_12x16(glyph.pixels(), output);
        display.draw_glyph_v2(p, ui::Size(12,16), style.foreground, style.background,output);
        return p.x()* zoom_level;
    }

    
    // 原始部分
    // const auto glyph = style.font.glyph(c);

    // display.draw_glyph(p, glyph, style.foreground, style.background, zoom_level);
    // return glyph.advance().x() * zoom_level;
}



static int draw_string_orgin(Point p,const Font& font,Color foreground,Color background,std::string_view text) {
    bool escape = false;
    size_t width = 0;
    Color pen = foreground;
    for (auto c : text) {
        if (escape) {
            if (c < std::size(term_colors))
                pen = term_colors[(uint8_t)c];
            else
                pen = foreground;
            escape = false;
        } else {
            if (c == '\x1B') {
                escape = true;
            } else {
                const auto glyph = font.glyph(c);
                display.draw_glyph(p, glyph, pen, background);
                const auto advance = glyph.advance();
                p += advance;
                width += advance.x();
            }
        }
    }
    return width;
}

int Painter::draw_string(Point p, const Style& style, std::string_view text) {
    return draw_string(p, style.font, style.foreground, style.background, text);
}


/// @brief 这个函数的目的是在于适配text窗体的字体大小
/// @param p 
/// @param style 
/// @param text 
/// @param fit_size 
/// @return 
int Painter::draw_string_with_fitsize(Point p, const Style& style, std::string_view text,int fit_size)
{
    // 这里进行最原始的渲染
    if(fit_size == 0)
    {
        return draw_string_orgin(p, style.font, style.foreground, style.background, text);
    }
    else
    {
        return draw_string(p, style.font, style.foreground, style.background, text);
    }   
}

int Painter::draw_string(
    Point p,
    const Font& font,
    Color foreground,
    Color background,
    std::string_view text) {
    bool escape = false;
    size_t width = 0;
    Color pen = foreground;

    for (auto c : text) {
        if (escape) {
            if (c < std::size(term_colors))
                pen = term_colors[(uint8_t)c];
            else
                pen = foreground;
            escape = false;
        } else {
            if (c == '\x1B') {
                escape = true;
            } else {

                if( c >= 0x20 && c<=0x7E)
                {
                    int tt_width = 12;
                    uint8_t idx = c-0x20;
                    
                    // uint16_t test_char[]=
                    // {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x079E, 0x010C, 0x0108, 0x0118, 0x0090, 0x0090, 0x00B0, 0x0060, 0x0060, 0x0020, 0x0020, 0x0020, 0x0014, 0x000C}
                    // ;
                    // display.draw_glyph_v2(p, ui::Size(12,24), foreground,background,(void *) test_char);
                    display.draw_glyph_v2(p, ui::Size(12,24), foreground,background,(void *) font12x24[idx]);
                    p += Point(tt_width,0);
                    width+=tt_width;
                }
                else
                {
                    const auto glyph = font.glyph(c);
                    uint16_t output[32];
                    convert_8x16_to_12x16(glyph.pixels(), output);
                    display.draw_glyph_v2(p, ui::Size(12,16), foreground, background,output);
                    p += Point(12,0);
                    width+=12;
                }

                // const auto glyph = font.glyph(c);
                // // uint8_t output[48];
                // // enlarge_8x16_to_16x24_saturated(glyph.pixels(), output);

                // // uint8_t output[32];
                // // enlarge_8x16_to_16x16_clean(glyph.pixels(), output);
                // // // enlarge_8x16_to_centered_16x16(glyph.pixels(), output);
                // // // enlarge_8x16_to_16x16_smooth(glyph.pixels(), output);
                // // Glyph n_glyph(16,16,output);
                // // uint8_t output[24];
                // // // uint16_t output[24];
                // // enlarge_8x16_to_8x24(glyph.pixels(), output);
                // // enlarge_8x16_to_12x16_spaced(glyph.pixels(), output);

                // uint16_t output[16];
                // convert_8x16_to_10x22(glyph.pixels(), output);
                // display.draw_glyph_v2(p, ui::Size(10,22), pen, background,output);
                // p += Point(10,0);
                // width+=10;
                // Glyph n_glyph(12,16,output);
                // const auto advance = n_glyph.advance();

                // 原始部分
                // display.draw_glyph(p, glyph, pen, background);
                // const auto advance = glyph.advance();

                // p += advance;
                // width += advance.x();
            }
        }
    }

    return width;
}

void Painter::draw_bitmap(Point p, const Bitmap& bitmap, Color foreground, Color background) {
    // If bright foreground colors on white background, darken the foreground color to improve visibility
    if ((background.v == ui::Color::white().v) && (foreground.to_greyscale() > 146))
        foreground = foreground.dark();

    display.draw_bitmap(p, bitmap.size, bitmap.data, foreground, background);
}

void Painter::draw_bitmap_with_autofit(Point p, const Bitmap& bitmap, Color background, Color foreground,int zoom)
{
    if ((background.v == ui::Color::white().v) && (foreground.to_greyscale() > 146))
        foreground = foreground.dark();

    display.draw_bitmap(p, bitmap.size, bitmap.data, foreground, background,zoom);
}

void Painter::draw_hline(Point p, int width, Color c) {
    display.fill_rectangle({p, {width, 1}}, c);
}

void Painter::draw_vline(Point p, int height, Color c) {
    display.fill_rectangle({p, {1, height}}, c);
}

void Painter::draw_rectangle(Rect r, Color c) {
    draw_hline(r.location(), r.width(), c);
    draw_vline({r.left(), r.top() + 1}, r.height() - 2, c);
    draw_vline({r.left() + r.width() - 1, r.top() + 1}, r.height() - 2, c);
    draw_hline({r.left(), r.top() + r.height() - 1}, r.width(), c);
}

void Painter::fill_rectangle(Rect r, Color c) {
    display.fill_rectangle(r, c);
}

void Painter::fill_rectangle_unrolled8(Rect r, Color c) {
    display.fill_rectangle_unrolled8(r, c);
}

void Painter::paint_widget_tree(Widget* w) {
    if (ui::is_dirty()) {
        paint_widget(w);
        ui::dirty_clear();
    }
}

void Painter::paint_widget(Widget* w) {
    if (w->hidden()) {
        // Mark widget (and all children) as invisible.
        w->visible(false);
    } else {
        // Mark this widget as visible and recurse.
        w->visible(true);

        if (w->dirty()) {
            w->paint(*this);
            // Force-paint all children.
            for (const auto child : w->children()) {
                child->set_dirty();
                paint_widget(child);
            }
            w->set_clean();
        } else {
            // Selectively paint all children.
            for (const auto child : w->children()) {
                paint_widget(child);
            }
        }
    }
}

} /* namespace ui */
