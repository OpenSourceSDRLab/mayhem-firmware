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

#ifndef __SIGGEN_H__
#define __SIGGEN_H__

#include "app_settings.hpp"
#include "radio_state.hpp"
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_transmitter.hpp"

#include "portapack.hpp"
#include "message.hpp"



namespace ui {

class SigGenView : public View {
   public:
    SigGenView(NavigationView& nav);
    ~SigGenView();

    void focus() override;

    std::string title() const override { return "Signal gen"; };

   private:
    int  FIT_OFFSET  = 320/8;
    void start_tx();
    void update_config();
    void update_tone();
    void on_tx_progress(const uint32_t progress, const bool done);

    TxRadioState radio_state_{
        0 /* frequency */,
        1750000 /* bandwidth */,
        1536000 /* sampling rate */
    };
    app_settings::SettingsManager settings_{
        "tx_siggen", app_settings::Mode::TX};

    const std::string shape_strings[6] = {// max 15 character text space.
                                          "Sine",
                                          "Triangle",
                                          "Saw up",
                                          "Saw down",
                                          "Square",
                                          "Pseudo Noise"};

    bool auto_update{false};

    Labels labels{
        {{ 0 , 0 }, "Modulation:", Theme::getInstance()->fg_light->foreground},
        {{ 0, ui::new_font_height * 1 }, "Shape:", Theme::getInstance()->fg_light->foreground},
        {{0, ui::new_font_height * 4}, "Tone:      Hz", Theme::getInstance()->fg_light->foreground,false},
        {{ ui::screen_width /2 + 4*ui::new_font_width,  9*ui::new_font_height}, "s.", Theme::getInstance()->fg_light->foreground}
    };

    OptionsField options_mod{
        {12 * ui::new_font_width, 0},
        12,
        {
            {"CW (No mod.)", 0},
            {"FM", 1},
            {"BPSK", 2},
            {"QPSK", 3},
            {"DSB", 4},
            {"AM 100% dep.", 5},
            {"AM 50% depth", 6},
            {"Pulse CW 25%", 7}
         },
         false,
         true
    };

    ImageOptionsField options_shape{
        { 7*ui::new_font_width, ui::new_font_height * 1, 32, 32},
        Theme::getInstance()->bg_darkest->foreground,
        Theme::getInstance()->bg_darkest->background,
        {{&bitmap_sig_sine, 0},
         {&bitmap_sig_tri, 1},
         {&bitmap_sig_saw_up, 2},
         {&bitmap_sig_saw_down, 3},
         {&bitmap_sig_square, 4},
         {&bitmap_sig_noise, 5}}};

    Text text_shape{
        { 12 * ui::new_font_width,ui::new_font_height * 1, 15 * ui::new_font_width, ui::new_font_height},
        "-",true
    };

    SymField symfield_tone{
        {5 * 8, ui::new_font_height * 4},
        5,
    };


    Button button_update{
        {0, 5*ui::new_font_height+4 , 8 * ui::new_font_width, 2*ui::new_font_height},
        "Update"};

    Checkbox checkbox_auto{
        {0, 8*ui::new_font_height},
        4,
        "Auto"};

    Checkbox checkbox_stop{
        {0, 9*ui::new_font_height},
        10,
        "Stop after"};

    NumberField field_stop{
        {ui::screen_width / 2 , 9*ui::new_font_height},
        2,
        {1, 99},
        1,
        ' '};

    TransmitterView tx_view{
        16 * 16,
        10000,
        12};

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* const p) {
            const auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            this->on_tx_progress(message.progress, message.done);
        }};
};

} /* namespace ui */

#endif /*__SIGGEN_H__*/
