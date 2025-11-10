/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * copyleft 2024 zxkmm AKA zix aka sommermorgentraum
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


int Painter::draw_char_source(Point p, const Style& style, char c, uint8_t zoom_level) {
    const auto glyph = style.font.glyph(c);

    display.draw_glyph(p, glyph, style.foreground, style.background, zoom_level);
    return glyph.advance().x() * zoom_level;
}


int Painter::draw_char(Point p, const Style& style, char c, uint8_t zoom_level) {
    if( c >= 0x20 && c<=0x7E)
    {
        uint8_t idx = c-0x20;
        display.draw_glyph_v2(p, ui::Size(12,24), style.foreground, style.background,(void *)font12x24[idx]);
        return p.x()* zoom_level;
    }
    else
    {
        return p.x()* zoom_level;
    }
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


/// @brief for fit new screen 
/// @param p source arg
/// @param style source arg
/// @param text source arg
/// @param fit_size 0 for old paint function or 1 for new paint function
/// @return 
int Painter::draw_string_with_fitsize(Point p, const Style& style, std::string_view text,int fit_size)
{
    // source
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
            } 
            else {
                if( c >= 0x20 && c<=0x7E)
                {
                    // new font width
                    int tt_width = 12;
                    // get new font idx
                    uint8_t idx = c-0x20;
                    // show new font
                    display.draw_glyph_v2(p, ui::Size(12,24), foreground,background,(void *) font12x24[idx]);
                    p += Point(tt_width,0);
                    width+=tt_width;
                }
            }
        }
    }
    return width;
}

int Painter::draw_string_source(
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
