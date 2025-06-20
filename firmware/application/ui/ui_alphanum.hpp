/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
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

#ifndef __ALPHANUM_H__
#define __ALPHANUM_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_painter.hpp"
#include "ui_textentry.hpp"
#include "ui_menu.hpp"

// TODO: Building this as a custom widget instead of using
// all the Button controls would save a considerable amount of RAM.
// The Buttons each have a backing string but these only need one char.

namespace ui {

// 这里是Text Entry框体入口
// 会有两个类共同操控此函数
class AlphanumView : public TextEntryView {
   public:
    AlphanumView(NavigationView& nav, std::string& str, size_t max_length, uint8_t enter_mode);

    AlphanumView(const AlphanumView&) = delete;
    AlphanumView(AlphanumView&&) = delete;
    AlphanumView& operator=(const AlphanumView&) = delete;
    AlphanumView& operator=(AlphanumView&&) = delete;

    bool on_encoder(const EncoderEvent delta) override;

   private:
    enum class ShiftMode : uint8_t {
        None,
        Shift,
        ShiftLock,
    };

    const char* const keys_lower = "abcdefghijklmnopqrstuvwxyz, .";
    const char* const keys_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ, .";
    const char* const keys_digit = "1234567890()'`\"+-*/=<>_\\!?, .";
    const char* const keys_symbl = "!@#$%^&*()[]'`\"{}|:;<>-_~?, .";
    const char* const keys_hex = "1234567890ABCDEF ";

    struct key_set_t {
        const char* name;
        const char* normal;
        const char* shifted;
    };

    const key_set_t key_sets[3] = {
        {"abc", keys_lower, keys_upper},
        {"123", keys_digit, keys_symbl},
        {"hex", keys_hex, keys_hex}};

    int16_t focused_button = 0;
    uint32_t mode = 0;  // Letters.
    ShiftMode shift_mode = ShiftMode::None;

    void set_mode(uint32_t new_mode, ShiftMode new_shift_mode = ShiftMode::None);
    void refresh_keys();
    void on_button(Button& button);

    std::array<Button, 29> buttons{};

    NewButton button_shift{
        {192 + 64, 10 * ui::new_font_height - 26, screen_width / 5, 38},
        {},
        &bitmap_icon_shift,
        Theme::getInstance()->bg_dark->background,
        /*vcenter*/ true
    };


    Labels labels{
        {{1 * ui::new_font_width, 13 * ui::new_font_height}, "Raw:", Theme::getInstance()->fg_light->foreground},
        {{1 * ui::new_font_width, 14 * ui::new_font_height}, "AKA:", Theme::getInstance()->fg_light->foreground}};

    NumberField field_raw{
        {5 * ui::new_font_width, 13 * ui::new_font_height},
        3,
        {1, 255},
        1,
        '0'
    };

    Text text_raw_to_char{
        {5 * ui::new_font_width, 14 * ui::new_font_height, 4 * ui::new_font_width, ui::new_font_height},
        "0",
        true
    };

    Button button_delete{
        {7 * ui::new_font_width, 16 * ui::new_font_height, 7 * ui::new_font_width, 2* ui::new_font_height},
        "<DEL"};

    Button button_mode{
        {14 * ui::new_font_width, 16* ui::new_font_height, 6 * ui::new_font_width, 2* ui::new_font_height},
        ""};
};

} /* namespace ui */

#endif /*__ALPHANUM_H__*/
