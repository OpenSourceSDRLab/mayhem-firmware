/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2017 Furrtek
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

#include "ui.hpp"
#include "ui_textentry.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_transmitter.hpp"

#include "message.hpp"
#include "modems.hpp"
#include "transmitter_model.hpp"
#include "app_settings.hpp"
#include "radio_state.hpp"
#include "portapack.hpp"

namespace ui {

class APRSTXView : public View {
   public:
    APRSTXView(NavigationView& nav);
    ~APRSTXView();

    void focus() override;

    std::string title() const override { return "APRS TX"; };

   private:
    TxRadioState radio_state_{
        144390000 /* frequency */,
        1750000 /* bandwidth */,
        AFSK_TX_SAMPLERATE /* sampling rate */
    };
    app_settings::SettingsManager settings_{
        "tx_aprs", app_settings::Mode::TX};

    std::string payload{""};

    void start_tx();
    void generate_frame();
    void generate_frame_pos();
    void on_tx_progress(const uint32_t progress, const bool done);

    Labels labels{
<<<<<<< HEAD
        {{UI_POS_X(0), 1 * 16}, "Source:       SSID:", Theme::getInstance()->fg_light->foreground},  // 6 alphanum + SSID
        {{UI_POS_X(0), 2 * 16}, " Dest.:       SSID:", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), 4 * 16}, "Info field:", Theme::getInstance()->fg_light->foreground},
=======
        {{0 * 8, 1 * ui::new_font_height}, "Source:       SSID:", Theme::getInstance()->fg_light->foreground},  // 6 alphanum + SSID
        {{0 * 8, 2 * ui::new_font_height}, " Dest.:       SSID:", Theme::getInstance()->fg_light->foreground},
        {{0 * 8, 4 * ui::new_font_height}, "Info field:", Theme::getInstance()->fg_light->foreground},
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb
    };


    SymField sym_source{
        {7 * ui::new_font_width, 1 * ui::new_font_height},
        6,
        SymField::Type::Alpha};

    NumberField num_ssid_source{
        {19 * ui::new_font_width, 1 * ui::new_font_height},
        2,
        {0, 15},
        1,
        ' '};

    SymField sym_dest{
        {7 * ui::new_font_width, 2 * ui::new_font_height},
        6,
        SymField::Type::Alpha
    };

    NumberField num_ssid_dest{
        {19 * ui::new_font_width, 2 * ui::new_font_height},
        2,
        {0, 15},
        1,
        ' '};

    Text text_payload{
<<<<<<< HEAD
        {UI_POS_X(0), 5 * 16, screen_width, 16},
        "-"};
    Button button_set{
        {UI_POS_X(0), 6 * 16, 80, 32},
        "Set"};
=======
        {0 * 8, 5 * ui::new_font_height, screen_width, ui::new_font_height},
        "-",
        true
    };


    Button button_set{
        {0 * 8, 6 * ui::new_font_height, 80, ui::new_font_height*2},
        "Set"
    };
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    TransmitterView tx_view{
        (int16_t)UI_POS_Y_BOTTOM(4),
        5000,
        0  // disable setting bandwith, since APRS used fixed 10k bandwidth
    };

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* const p) {
            const auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            this->on_tx_progress(message.progress, message.done);
        }};
};

} /* namespace ui */
