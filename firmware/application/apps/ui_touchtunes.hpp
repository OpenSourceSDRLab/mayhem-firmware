/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2017 Furrtek
 * Copyright (C) 2022 NotPike
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
#include "ui_transmitter.hpp"
#include "transmitter_model.hpp"
#include "radio_state.hpp"
#include "encoders.hpp"

// The coding in notpike's script is quite complex, using multiple LUTs to form the data sent to the YSO.
// The format is actually very simple if it is rather seen as short and long gaps between pulses (as seen in many OOK remotes).
// The frames and data rate suspiciously match the NEC infrared protocol (http://www.sbprojects.com/knowledge/ir/nec.php) without
// the address complement. The exact data rate would be 1786 baud (560us/fragment).
// NotPike: The data rate should be 1786 but the remote was transmitting at 1766

// Pin 0 - On/Off
// ffff00 a2888a2aaaa8888aa2aa2220
// 1010 0010 1000 1000 1000 1010 0010 1010 1010 1010 1010 1000 1000 1000 1000 1010 1010 0010 1010 1010 0010 0010 0010 0000
// 101000101000100010001010001010101010101010101000100010001000101010100010101010100010001000100000
// S L   S L   L   L   S L   S S S S S S S S S L   L   L   L   S S S L   S S S S L   L   L

// Pin 1 - On/Off
// ffff00 a2888a22aaaa2222a8aa8888
// 1010 0010 1000 1000 1000 1010 0010 0010 1010 1010 1010 1010 0010 0010 0010 0010 1010 1000 1010 1010 1000 1000 1000 1000
// 101000101000100010001010001000101010101010101010001000100010001010101000101010101000100010001000
// S L   S L   L   L   S L   L   S S S S S S S S L   L   L   L   S S S L   S S S S L   L   L

// Pin 2 - On/Off
// ffff00 a2888a28aaaa2222a8aa8888
// 1010 0010 1000 1000 1000 1010 0010 1000 1010 1010 1010 1010 0010 0010 0010 0010 1010 1000 1010 1010 1000 1000 1000 1000
// 101000101000100010001010001010001010101010101010001000100010001010101000101010101000100010001000
// S L   S L   L   L   S L   S L   S S S S S S S L   L   L   L   S S S L   S S S S L   L   L

// Pin 3 - On/Off
// ffff00 a2888a222aaa8888aa2aa222
// 1010 0010 1000 1000 1000 1010 0010 0010 0010 1010 1010 1010 1000 1000 1000 1000 1010 1010 0010 1010 1010 0010 0010 0010 0000 0000
// 10100010100010001000101000100010001010101010101010001000100010001010101000101010101000100010001000
// S L   S L   L   L   S L   L   L   S S S S S S S L   L   L   L   S S S L   S S S S L   L   L

// The sync word seems to be SLSLLLSL (01011101, or 0xBA reversed)
// The pin # is sent LSB first
// The button data seems to be SLLLLSSSLSSSSLLL (0111100010000111 and a terminating pulse, or 0x7887 reversed)
// The hex data only seems scrambled because of the shift induced by the short or long gaps (10 or 1000)
// The radio frame's duration depends on the value of the bits

#define TOUCHTUNES_MAX_PIN 255
#define TOUCHTUNES_REPEATS 4
#define TOUCHTUNES_SYNC_WORD 0x5D

static int  FIT_OFFSET = 320/8;

// Each 16bit button code is actually 8bit followed by its complement
const uint8_t button_codes[32] = {
    0x32,  // Pause
    0x78,  // On/Off
    0x70,  // P1
    0x60,  // P2
    0xCA,  // P3
    0x20,  // F1
    0xF2,  // Up
    0xA0,  // F2
    0x84,  // Left
    0x44,  // OK
    0xC4,  // Right
    0x30,  // F3
    0x80,  // Down
    0xB0,  // F4
    0xF0,  // 1
    0x08,  // 2
    0x88,  // 3
    0x48,  // 4
    0xC8,  // 5
    0x28,  // 6
    0xA8,  // 7
    0x68,  // 8
    0xE8,  // 9
    0x18,  // Music_Karaoke
    0x98,  // 0
    0x58,  // Lock_Queue
    0xD0,  // Zone 1 Vol+
    0x90,  // Zone 2 Vol+
    0xC0,  // Zone 3 Vol+
    0x50,  // Zone 1 Vol-
    0x10,  // Zone 2 Vol-
    0x40,  // Zone 3 Vol-
};

namespace ui {

class TouchTunesView : public View {
   public:
    TouchTunesView(NavigationView& nav);
    ~TouchTunesView();

    void focus() override;

    std::string title() const override { return "TouchTunes"; };

   private:
    TxRadioState radio_state_{
        433920000 /* frequency */,
        1750000 /* bandwidth */,
        OOK_SAMPLERATE /* sampling rate */
    };

    uint32_t scan_button_index{};
    uint32_t pin{0};

    enum tx_modes {
        IDLE = 0,
        SINGLE,
        SCAN
    };

    tx_modes tx_mode = IDLE;

    void start_tx(const uint32_t button_index);
    void stop_tx();
    void on_tx_progress(const uint32_t progress, const bool done);
    void start_ew();
    void stop_ew();

    struct remote_layout_t {
        Point position;
        std::string text;
    };

    

    const std::array<remote_layout_t, 32> remote_layout{{
                                                            {{ui::screen_width - (ui::new_font_width*7)*2 , 0}, "PAUSE"},
                                                            {{ui::screen_width - (ui::new_font_width*7)*1, 0}, "POWER"},
                                                         
                                                            {{0 * ui::new_font_width, 2 *ui::new_font_height + 16}, "1"},
                                                            {{4 * ui::new_font_width, 2 *ui::new_font_height + 16}, "2"},
                                                            {{8 * ui::new_font_width, 2 *ui::new_font_height + 16}, "3"},
                                                            {{14 * ui::new_font_width, 2 *ui::new_font_height + 16}, "P1"},
                                                            {{18 * ui::new_font_width, 2 *ui::new_font_height + 16}, "P2"},
                                                            {{22 * ui::new_font_width, 2 *ui::new_font_height + 16}, "P3"},

                                                            {{0 * ui::new_font_width, 4 *ui::new_font_height + 16}, "4"},
                                                            {{4 * ui::new_font_width, 4 *ui::new_font_height + 16}, "5"},
                                                            {{8 * ui::new_font_width, 4 *ui::new_font_height + 16}, "6"},
                                                            {{14 * ui::new_font_width, 4 *ui::new_font_height + 16}, "F1"},
                                                            {{18 * ui::new_font_width +4, 4 *ui::new_font_height + 16}, "^"},
                                                            {{22 * ui::new_font_width, 4 *ui::new_font_height + 16}, "F2"},
                                                        
                                                            {{0 * ui::new_font_width, 6 *ui::new_font_height + 16 }, "7"},
                                                            {{4 * ui::new_font_width, 6 *ui::new_font_height + 16}, "8"},
                                                            {{8 * ui::new_font_width, 6 *ui::new_font_height + 16}, "9"},
                                                            {{14 * ui::new_font_width, 6 *ui::new_font_height + 16}, "<"},
                                                            {{18 * ui::new_font_width, 6 *ui::new_font_height + 16}, "OK"},
                                                            {{22 * ui::new_font_width + 8, 6 *ui::new_font_height + 16}, ">"},
                                                            
                                                            {{0 * ui::new_font_width, 8 * ui::new_font_height + 16}, "*"},
                                                            {{4 * ui::new_font_width, 8 * ui::new_font_height + 16}, "0"},
                                                            {{8 * ui::new_font_width, 8 * ui::new_font_height + 16}, "#"},
                                                            {{14 * ui::new_font_width, 8 * ui::new_font_height + 16}, "F3"},
                                                            {{18 * ui::new_font_width+4, 8 * ui::new_font_height + 16}, "V"},
                                                            {{22 * ui::new_font_width, 8 * ui::new_font_height + 16}, "F4"},
                                                            
                                                            {{14 * ui::new_font_width, 10 * ui::new_font_height + 16}, "+"},
                                                            {{18 * ui::new_font_width, 10 * ui::new_font_height + 16}, "+"},
                                                            {{22 * ui::new_font_width, 10 * ui::new_font_height + 16}, "+"},

                                                            {{14 * ui::new_font_width, 14 * ui::new_font_height + 16}, "-"},
                                                            {{18 * ui::new_font_width, 14 * ui::new_font_height + 16} , "-"},
                                                            {{22 * ui::new_font_width, 14 * ui::new_font_height + 16}, "-"}}};
                                                         
                                                         
                                                         
                                                         
                                                         

    Labels labels{
        {{0 * 8, 0 * ui::new_font_height}, "PIN:", Theme::getInstance()->fg_light->foreground},
        {{12 * ui::new_font_width, 13 * ui::new_font_height + 8}, "VOL1 VOL2 VOL3", Theme::getInstance()->fg_light->foreground}
    
    };

    
    NumberField field_pin{
        {6 * ui::new_font_width, 0 * ui::new_font_height},
        3,
        {0, 255},
        1,
        '0'};

    std::array<Button, 32> buttons{};

    

    Checkbox check_scan{
        {2 * 8, ui::screen_height - 8*ui::new_font_height},
        4,
        "Scan"};

    Checkbox check_ew{
        {2 * 8, ui::screen_height - 6*ui::new_font_height},
        4,
        "EW Mode"};

    Text text_status{
        {2 * 8, ui::screen_height - 4*ui::new_font_height , 128, ui::new_font_height},
        "Ready",true};

    ProgressBar progressbar{
        // {2 * 8, 35 * 8, 208, 16}
        {2 * 8, ui::screen_height - 16 -ui::new_font_height , 320-2*8*2, 16}
    };

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* const p) {
            const auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            this->on_tx_progress(message.progress, message.done);
        }};
};

} /* namespace ui */
