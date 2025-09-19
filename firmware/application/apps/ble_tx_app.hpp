/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2017 Furrtek
 * Copyright (C) 2023 TJ Baginski
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

#ifndef __BLE_TX_APP_H__
#define __BLE_TX_APP_H__

#include "ui.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_transmitter.hpp"
#include "ui_text_editor.hpp"
#include "ui_freq_field.hpp"
#include "ui_record_view.hpp"
#include "app_settings.hpp"
#include "radio_state.hpp"
#include "replay_thread.hpp"
#include "log_file.hpp"
#include "utility.hpp"
#include "file_path.hpp"

#include "recent_entries.hpp"

#include <string>
#include <memory>

class BLELoggerTx {
   public:
    Optional<File::Error> append(const std::filesystem::path& filename) {
        return log_file.append(filename);
    }

    void log_raw_data(const std::string& data);

   private:
    LogFile log_file{};
};

namespace ui {

enum PKT_TYPE {
    PKT_TYPE_INVALID_TYPE,
    PKT_TYPE_RAW,
    PKT_TYPE_DISCOVERY,
    PKT_TYPE_IBEACON,
    PKT_TYPE_ADV_IND,
    PKT_TYPE_ADV_DIRECT_IND,
    PKT_TYPE_ADV_NONCONN_IND,
    PKT_TYPE_ADV_SCAN_IND,
    PKT_TYPE_SCAN_REQ,
    PKT_TYPE_SCAN_RSP,
    PKT_TYPE_CONNECT_REQ,
    PKT_TYPE_LL_DATA,
    PKT_TYPE_LL_CONNECTION_UPDATE_REQ,
    PKT_TYPE_LL_CHANNEL_MAP_REQ,
    PKT_TYPE_LL_TERMINATE_IND,
    PKT_TYPE_LL_ENC_REQ,
    PKT_TYPE_LL_ENC_RSP,
    PKT_TYPE_LL_START_ENC_REQ,
    PKT_TYPE_LL_START_ENC_RSP,
    PKT_TYPE_LL_UNKNOWN_RSP,
    PKT_TYPE_LL_FEATURE_REQ,
    PKT_TYPE_LL_FEATURE_RSP,
    PKT_TYPE_LL_PAUSE_ENC_REQ,
    PKT_TYPE_LL_PAUSE_ENC_RSP,
    PKT_TYPE_LL_VERSION_IND,
    PKT_TYPE_LL_REJECT_IND,
    PKT_TYPE_NUM_PKT_TYPE
};

struct BLETxPacket {
    char macAddress[13];
    char advertisementData[63];
    char packetCount[11];
    uint32_t packet_count;
    PKT_TYPE packetType;
};

class BLETxView : public View {
   public:
    BLETxView(NavigationView& nav);
    BLETxView(NavigationView& nav, BLETxPacket packet);
    ~BLETxView();

    void set_parent_rect(const Rect new_parent_rect) override;
    void paint(Painter&) override{};

    void focus() override;

    bool is_active() const;
    void toggle();
    void start();
    void stop();
    void reset();
    void handle_replay_thread_done(const uint32_t return_code);
    void file_error();
    bool saveFile(const std::filesystem::path& path);

    std::string title() const override { return "BLE TX"; };

   private:
    void on_timer();
    void on_data(uint32_t value, bool is_data);
    void on_file_changed(const std::filesystem::path& new_file_path);
    void on_save_file(const std::string value);
    void on_tx_progress(const bool done);
    void on_random_data_change(std::string value);
    void update_current_packet(BLETxPacket packet, uint32_t currentIndex);

    NavigationView& nav_;
    TxRadioState radio_state_{
        2'402'000'000 /* frequency */,
        4'000'000 /* bandwidth */,
        4'000'000 /* sampling rate */
    };
    app_settings::SettingsManager settings_{
        "tx_ble", app_settings::Mode::TX};

    uint8_t console_color{0};
    uint32_t prev_value{0};

    std::filesystem::path file_path{};
    std::filesystem::path packet_save_path{bletx_dir / u"BLETX_????.TXT"};
    uint8_t channel_number = 37;
    bool auto_channel = false;

    char randomMac[13] = "010203040506";

    bool is_running = false;

    int16_t timer_count{0};
    int16_t timer_period{1};
    int16_t auto_channel_counter = 0;
    int16_t auto_channel_period{6};

    bool repeatLoop = false;
    uint32_t packet_counter{0};
    uint32_t num_packets{0};
    uint32_t current_packet{0};
    bool random_mac = false;
    bool file_override = false;

    typedef struct {
        uint16_t line;
        uint16_t col;
    } CursorPos;

    std::unique_ptr<FileWrapper> dataFileWrapper{};
    File dataFile{};
    std::filesystem::path dataTempFilePath{bletx_dir / u"dataFileTemp.TXT"};
    std::vector<uint16_t> markedBytes{};
    CursorPos cursor_pos{};
    uint8_t marked_counter = 0;

    static constexpr uint8_t mac_address_size_str{12};
    static constexpr uint8_t max_packet_size_str{62};
    static constexpr uint8_t max_packet_repeat_str{10};
    static constexpr uint32_t max_packet_repeat_count{UINT32_MAX};
    static constexpr uint32_t max_num_packets{32};

    BLETxPacket packets[max_num_packets];

    PKT_TYPE pduType = {PKT_TYPE_DISCOVERY};

    static constexpr auto header_height = 10 * 16;
    static constexpr auto switch_button_height = 6 * 16;

    // 第1行开始

    // 采用小字布局
    Button button_open{
        {0 * 8, 0 * 16, 10 * 8, 2 * 16},
        "Open file",
        false,
        false
    };

    // 注意button占用两行，这是第1行
    Text text_filename{
        {12 * 8 , 0 * 16, 16 * 8, 16},
        "-",
        false
    };

    // 注意button占用两行，这是第2行
    ProgressBar progressbar{
        {12 * 8 , 1 * 16, 14 * 8, 16}};

    Checkbox check_rand_mac{
        {ui::screen_width - 10*8 , 1 * 16},
        6,
        "?? Mac",
        true,
        false};

    // 第1行结束

    // 第2行开始
    TxFrequencyField field_frequency{
        {0 * 8, 2 * 16},
        nav_,
        false};

    TransmitterView2 tx_view{
        // {11 * 8, 2 * 16},
        {11 * 8, 2 * 16},
        /*short_ui*/ true};

    Checkbox check_loop{
        {ui::screen_width - 8*10, 2 * 16},
        4,
        "Loop",
        true,
        false
    };

    ImageButton button_play{
        {screen_width - 2 * 8, 2 * 16, 2 * 8, 1 * 16},
        &bitmap_play,
        Theme::getInstance()->fg_green->foreground,
        Theme::getInstance()->fg_green->background
    };
    // 第2行结束

    // 第3行开始 -- 字体可以逐步大一点

    Labels label_speed{
        {{0 * 8, 4 * 16}, "Speed:", Theme::getInstance()->fg_light->foreground}};

    OptionsField options_speed{
        {7 * ui::new_font_width, 4 * 16},
        3,
        {{"1 ", 1},     // 16ms
         {"2 ", 2},     // 32ms
         {"3 ", 3},     // 48ms
         {"4 ", 6},     // 100ms
         {"5 ", 12}},
         false,
         true
    };  // 200ms

    OptionsField options_channel{
        {11 * ui::new_font_width, 4 * 16},
        5,
        {{"Ch.37 ", 37},
         {"Ch.38", 38},
         {"Ch.39", 39},
         {"Auto", 40}},
        false,true     
    };

    OptionsField options_adv_type{
        {17 * ui::new_font_width, 4 * 16},
        14,
        {{"DISCOVERY ", PKT_TYPE_DISCOVERY},
         {"ADV_IND", PKT_TYPE_ADV_IND},
         {"ADV_DIRECT", PKT_TYPE_ADV_DIRECT_IND},
         {"ADV_NONCONN", PKT_TYPE_ADV_NONCONN_IND},
         {"ADV_SCAN_IND", PKT_TYPE_ADV_SCAN_IND},
         {"SCAN_REQ", PKT_TYPE_SCAN_REQ},
         {"SCAN_RSP", PKT_TYPE_SCAN_RSP},
         {"CONNECT_REQ", PKT_TYPE_CONNECT_REQ}},
         false,false 
    
    };
    // 第3行结束

    //第4行开始
    Labels label_marked_data{
        {{0 * 8, 4 * 16 + ui::new_font_height * 1}, "Marked Data:", Theme::getInstance()->fg_light->foreground}};

    OptionsField marked_data_sequence{
        // {12 * 8, 8 * 8},
        {12 * ui::new_font_width , 4 * 16 + ui::new_font_height * 1},
        8,
        {{"Ascend", 0},
         {"Descend", 1},
         {"Random", 2}},
        false,
        true
    };
    //第4行结束


    Labels label_packet_index{
        {{0 * 8, 4 * 16 + ui::new_font_height * 3}, "Packet Index:", Theme::getInstance()->fg_light->foreground}};

    Text text_packet_index{
        {13 * ui::new_font_width , 4 * 16 + ui::new_font_height * 3 , 12 * ui::new_font_width, ui::new_font_height},
        "-",true};

    Labels label_packets_sent{
        {{0 * 8, 4 * 16 + ui::new_font_height * 4}, "Repeat Count:", Theme::getInstance()->fg_light->foreground}};

    Text text_packets_sent{
        {13 * ui::new_font_width , 4 * 16 + ui::new_font_height * 4, 12 * ui::new_font_width, ui::new_font_height},
        "-",true};

    Labels label_mac_address{
        {{0 * 8, 4 * 16 + ui::new_font_height * 5}, "Mac Address:", Theme::getInstance()->fg_light->foreground}};

    Text text_mac_address{
        {12 * ui::new_font_width, 4 * 16 + ui::new_font_height * 5, 20 * ui::new_font_width, ui::new_font_height},
        "-",true};


    Labels label_data_packet{
        {{0 * 8, 4 * 16 + ui::new_font_height * 6}, "Packet Data:", Theme::getInstance()->fg_light->foreground}};

    Console console{
        {0, 4 * 16 + ui::new_font_height * 7, screen_width, ui::new_font_height*4}};


    TextViewer dataEditView{
        {0, 4 * 16 + ui::new_font_height * 7, screen_width, ui::new_font_height*4}};


    Button button_clear_marked{
        // {1 * 8, 14 * 16, 13 * 8, 3 * 8},
        { 0 , ui::screen_height - ui::new_font_height * 5, 13 * ui::new_font_width, ui::new_font_height},
        "Clear Marked"
    };

    Button button_save_packet{
        // {1 * 8, 16 * 16, 13 * 8, 2 * 16},
        {0 , ui::screen_height - ui::new_font_height *3, 13 * ui::new_font_width, ui::new_font_height*2},
        "Save Packet"};

    Button button_switch{
        // {16 * 8, 16 * 16, 13 * 8, 2 * 16},

        {ui::screen_width/2, ui::screen_height - ui::new_font_height *3, 13 * ui::new_font_width, ui::new_font_height*2},
        "Switch to Rx"};

    std::string str_log{""};
    bool logging{true};
    bool logging_done{false};
    std::string packetFileBuffer{};

    std::unique_ptr<BLELoggerTx> logger{};

    MessageHandlerRegistration message_handler_packet{
        Message::ID::AFSKData,
        [this](Message* const p) {
            const auto message = static_cast<const AFSKDataMessage*>(p);
            this->on_data(message->value, message->is_data);
        }};

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* const p) {
            const auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            this->on_tx_progress(message.done);
        }};

    MessageHandlerRegistration message_handler_frame_sync{
        Message::ID::DisplayFrameSync,
        [this](const Message* const) {
            this->on_timer();
        }};
};

} /* namespace ui */

#endif /*__UI_AFSK_RX_H__*/
