/*
 * Copyright (C) 2014 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2017 Furrtek
 * Copyright (C) 2023 TJ Baginski
 * Copyright (C) 2025 Tommaso Ventafridda
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

#ifndef __BLE_RX_APP_H__
#define __BLE_RX_APP_H__

#include "ble_tx_app.hpp"

#include "ui.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_freq_field.hpp"
#include "ui_record_view.hpp"
#include "app_settings.hpp"
#include "radio_state.hpp"
#include "database.hpp"
#include "log_file.hpp"
#include "utility.hpp"
#include "usb_serial_thread.hpp"
#include "file_path.hpp"

#include "recent_entries.hpp"

class BLELogger {
   public:
    Optional<File::Error> append(const std::filesystem::path& filename) {
        return log_file.append(filename);
    }

    void log_raw_data(const std::string& data);

   private:
    LogFile log_file{};
};

namespace ui {
typedef enum {
    ADV_IND = 0,
    ADV_DIRECT_IND = 1,
    ADV_NONCONN_IND = 2,
    SCAN_REQ = 3,
    SCAN_RSP = 4,
    CONNECT_REQ = 5,
    ADV_SCAN_IND = 6,
    RESERVED0 = 7,
    RESERVED1 = 8,
    RESERVED2 = 9,
    RESERVED3 = 10,
    RESERVED4 = 11,
    RESERVED5 = 12,
    RESERVED6 = 13,
    RESERVED7 = 14,
    RESERVED8 = 15,
    UNKNOWN = 16
} ADV_PDU_TYPE;

typedef enum {
    MAC_VENDOR_UNKNOWN = 0,
    MAC_VENDOR_FOUND = 1,
    MAC_VENDOR_NOT_FOUND = 2,
    MAC_DB_NOT_FOUND = 3
} MAC_VENDOR_STATUS;

struct BleRecentEntry {
    using Key = uint64_t;

    static constexpr Key invalid_key = 0xFFFFFFFFFFFFF;

    uint64_t uniqueKey;
    int dbValue;
    BlePacketData packetData;
    std::string timestamp;
    std::string dataString;
    std::string nameString;
    std::string informationString;
    bool include_name;
    uint16_t numHits;
    ADV_PDU_TYPE pduType;
    uint8_t channelNumber;
    MAC_VENDOR_STATUS vendor_status;
    std::string vendor_name;
    bool entryFound;

    BleRecentEntry()
        : BleRecentEntry{0} {
    }

    BleRecentEntry(
        const uint64_t uniqueKey)
        : uniqueKey{uniqueKey},
          dbValue{},
          packetData{},
          timestamp{},
          dataString{},
          nameString{},
          informationString{},
          include_name{},
          numHits{},
          pduType{},
          channelNumber{},
          vendor_status{MAC_VENDOR_UNKNOWN},
          vendor_name{},
          entryFound{} {
    }

    Key key() const {
        return uniqueKey;
    }
};

using BleRecentEntries = RecentEntries<BleRecentEntry>;
using BleRecentEntriesView = RecentEntriesView<BleRecentEntries>;

class BleRecentEntryDetailView : public View {
   public:
    BleRecentEntryDetailView(NavigationView& nav, const BleRecentEntry& entry);

    void set_entry(const BleRecentEntry& new_entry);
    const BleRecentEntry& entry() const { return entry_; };

    void update_data();
    void focus() override;
    void paint(Painter&) override;
    static BLETxPacket build_packet(BleRecentEntry entry_);

   private:
    NavigationView& nav_;
    BleRecentEntry entry_{};
    void on_save_file(const std::string value, BLETxPacket packetToSave);
    bool saveFile(const std::filesystem::path& path, BLETxPacket packetToSave);
    std::string packetFileBuffer{};
    std::filesystem::path packet_save_path{blerx_dir / u"Lists/????.csv"};

    static constexpr uint8_t total_data_lines{5};

    Labels label_mac_address{
        {{UI_POS_X(0), UI_POS_Y(0)}, "Mac Address:", Theme::getInstance()->fg_light->foreground}};

    Text text_mac_address{
        {UI_POS_X(12), UI_POS_Y(0), UI_POS_WIDTH(17), UI_POS_HEIGHT(1)},
        "-"};

    Labels label_pdu_type{
        {{UI_POS_X(0), UI_POS_Y(1)}, "PDU Type:", Theme::getInstance()->fg_light->foreground}};

    Text text_pdu_type{
        {9 * 8, UI_POS_Y(1), 17 * 8, UI_POS_HEIGHT(1)},
        "-"};

    Labels label_vendor{
        {{UI_POS_X(0), UI_POS_Y(2)}, "Vendor:", Theme::getInstance()->fg_light->foreground}};

    Text text_vendor{
        {7 * 8, UI_POS_Y(2), 23 * 8, UI_POS_HEIGHT(1)},
        "-"};

    Labels labels{
        {{UI_POS_X(0), UI_POS_Y(3)}, "Len", Theme::getInstance()->fg_light->foreground},
        {{5 * 8, UI_POS_Y(3)}, "Type", Theme::getInstance()->fg_light->foreground},
        {{10 * 8, UI_POS_Y(3)}, "Value", Theme::getInstance()->fg_light->foreground},
    };

    Button button_send{
        {UI_POS_X_CENTER(12) - UI_POS_WIDTH(8), 224, 96, 24},
        "Send"};

    Button button_done{
        {UI_POS_X_CENTER(12) + UI_POS_WIDTH(8), 224, 96, 24},
        "Done"};

    Button button_save{
        {UI_POS_X_CENTER(12), 264, 96, 24},
        "Save"};

    bool send_updates{false};

    Rect draw_field(
        Painter& painter,
        const Rect& draw_rect,
        const Style& style,
        const std::string& label,
        const std::string& value);
};

class BLERxView : public View {
   public:
    BLERxView(NavigationView& nav);
    ~BLERxView();

    void set_parent_rect(const Rect new_parent_rect) override;
    void paint(Painter&) override{};

    void focus() override;

    std::string title() const override { return "BLE RX"; };

   private:
    std::string build_line_str(BleRecentEntry entry);
    void on_save_file(const std::string value);
    bool saveFile(const std::filesystem::path& path);
    std::unique_ptr<UsbSerialThread> usb_serial_thread{};
    void on_data(BlePacketData* packetData);
    void log_ble_packet(BlePacketData* packet);
    void on_filter_change(std::string value);
    void on_file_changed(const std::filesystem::path& new_file_path);
    void file_error();
    void on_timer();
    void handle_entries_sort(uint8_t index);
    bool handle_filter_options(uint8_t index, const BleRecentEntry& entry);
    bool updateEntry(const BlePacketData* packet, BleRecentEntry& entry, ADV_PDU_TYPE pdu_type);
    bool parse_beacon_data(const uint8_t* data, uint8_t length, std::string& nameString, std::string& informationString);
    bool parse_tracking_beacon_data(const uint8_t* data, uint8_t length, std::string& nameString, std::string& informationString);

    NavigationView& nav_;

    RxRadioState radio_state_{
        2402000000 /* frequency */,
        4000000 /* bandwidth */,
        4000000 /* sampling rate */,
        ReceiverModel::Mode::WidebandFMAudio};

    uint8_t channel_index{0};
    uint8_t sort_index{0};
    uint8_t filter_index{0};
    bool uniqueParsing = false;
    bool duplicatePackets = false;
    std::string filter{};
    bool logging{false};
    bool serial_logging{false};
    bool async_tx_states_when_entered{false};

    bool name_enable{true};
    app_settings::SettingsManager settings_{
        "rx_ble",
        app_settings::Mode::RX,
        {
            {"channel_index"sv, &channel_index},
            {"sort_index"sv, &sort_index},
            {"filter"sv, &filter},
            {"log"sv, &logging},
            {"filter_index"sv, &filter_index},
            // disabled to always start without USB serial activated until we can make it non blocking if not connected
            // {"serial_log"sv, &serial_logging},
            {"name"sv, &name_enable},
            {"unique_parsing"sv, &uniqueParsing},
            {"duplicate_packets"sv, &duplicatePackets},
        }};

    std::string str_console = "";
    uint8_t console_color{0};
    uint32_t prev_value{0};
    uint8_t channel_number = 37;
    bool auto_channel = false;

    int16_t timer_count{0};
    int16_t timer_period{1};  // 25ms

    std::string filterBuffer{};
    std::string listFileBuffer{};
    std::string headerStr = "Timestamp, MAC Address, Name, Packet Type, Data, Hits, dB, Channel";
    uint16_t maxLineLength = 140;

    std::filesystem::path file_path{};
    uint64_t found_count = 0;
    uint64_t total_count = 0;
    std::vector<std::string> searchList{};

    std::filesystem::path find_packet_path{blerx_dir / u"Find/????.TXT"};
    std::filesystem::path log_packets_path{blerx_dir / u"Logs/????.TXT"};
    std::filesystem::path packet_save_path{blerx_dir / u"Lists/????.csv"};

<<<<<<< HEAD
    static constexpr auto header_height = 12 * 8;
    static constexpr auto switch_button_height = 3 * 16;
=======
    // 动态渲染区域要求不遮盖已有行
    static constexpr auto header_height = 4 * 24;
    static constexpr auto switch_button_height = 24;
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    OptionsField options_channel{
        {UI_POS_X(0), UI_POS_Y(0)},
        5,
        {
            {"Ch.37", 37},
            {"Ch.38", 38},
            {"Ch.39", 39},
            {"Auto", 40}
        },
        false,
        true,
    };

    RxFrequencyField field_frequency{
<<<<<<< HEAD
        {UI_POS_X(6), UI_POS_Y(0)},
        nav_};

    RFAmpField field_rf_amp{
        {UI_POS_X(16), UI_POS_Y(0)}};

    LNAGainField field_lna{
        {UI_POS_X(18), UI_POS_Y(0)}};

    VGAGainField field_vga{
        {UI_POS_X(21), UI_POS_Y(0)}};

    RSSI rssi{
        {UI_POS_X(24), 0, UI_POS_WIDTH_REMAINING(6), 4}};

    Channel channel{
        {UI_POS_X(24), 5, UI_POS_WIDTH_REMAINING(6), 4}};

    Labels label_sort{
        {{UI_POS_X(0), UI_POS_Y(1)}, "Sort:", Theme::getInstance()->fg_light->foreground}};

    OptionsField options_sort{
        {5 * 8, UI_POS_Y(1)},
        4,
        {{"MAC", 0},
         {"Hits", 1},
         {"dB", 2},
         {"Time", 3},
         {"Name", 4},
         {"Info", 5}}};

    Button button_filter{
        {11 * 8, UI_POS_Y(1), 7 * 8, 16},
        "Filter:"};

    OptionsField options_filter{
        {18 * 8 + 2, UI_POS_Y(1)},
        7,
        {{"Data", 0},
         {"MAC", 1},
         {"Name", 2},
         {"Info", 3},
         {"Vendor", 4},
         {"Channel", 5}}};
=======
        {6 * ui::new_font_width, 0 * ui::new_font_height},
        nav_};

    RFAmpField field_rf_amp{
        {16 * ui::new_font_width, 0 * ui::new_font_height}};

    LNAGainField field_lna{
        {18 * ui::new_font_width, 0 * ui::new_font_height}};

    VGAGainField field_vga{
        {21 * ui::new_font_width ,0 * ui::new_font_height}};

    RSSI rssi{
        {24 * ui::new_font_width, 0, 4 * 8, 8}
    };

    Channel channel{
        { 24 * ui::new_font_width, 9, 4 * 8, 8}
    };

    Labels label_sort{
        {
            {0 * 8, 1 * ui::new_font_height}, 
            "Sort:", 
            Theme::getInstance()->fg_light->foreground
        }
    };

    OptionsField options_sort{
        {5 * ui::new_font_width, 1 * ui::new_font_height},
        4,
        {
            {"MAC", 0},
            {"Hits", 1},
            {"dB", 2},
            {"Time", 3},
            {"Name", 4}
        },
        false,
        true
    };

    Button button_filter{
        {11 * ui::new_font_width, 1 * ui::new_font_height, 7 * ui::new_font_width, ui::new_font_height},
        "Filter:"};

    OptionsField options_filter{
        {18 * ui::new_font_width + 2, 1 * ui::new_font_height},
        4,
        {
            {"Data", 0},
            {"MAC", 1}
        },
        false,
        true
    };
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    Checkbox check_log{
        {10 * ui::new_font_width, 2 * ui::new_font_height},
        3,
        "Log",
<<<<<<< HEAD
        true};

    Checkbox check_name{
        {UI_POS_X(0), 4 * 8 + 2},
        3,
        "Name",
        true};
=======
        true,false};
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    Checkbox check_serial_log{
        {18 * ui::new_font_width + 2, 2 * ui::new_font_height},
        7,
        "USB Log",
        true,false
    };

<<<<<<< HEAD
    Checkbox check_unique{
        {0 * 8 + 2, 7 * 8 + 2},
        7,
        "Unique",
        true};

    Checkbox check_duplicate_packets{
        {10 * 8 + 2, 7 * 8 + 2},
        7,
        "Duplicate",
        true};

    Button button_find{
        {UI_POS_X(0), 10 * 8 - 2, 4 * 8, 16},
        "Find"};

    Labels label_found{
        {{5 * 8, 10 * 8 - 2}, "Found:", Theme::getInstance()->fg_light->foreground}};

    Text text_found_count{
        {11 * 8, 10 * 8 - 2, 20 * 8, 16},
        "0/0"};
=======
    Checkbox check_name{
        {0 * ui::new_font_width, 2 * ui::new_font_height},
        3,
        "Name",
        true,false
    };

    Button button_find{
        {0 * 8, 3 * ui::new_font_height, 4 * ui::new_font_width, ui::new_font_height},
        "Find"
    };

    Labels label_found{
        {{5 * ui::new_font_width, 3* ui::new_font_height}, "Found:", Theme::getInstance()->fg_light->foreground}
    };

    Text text_found_count{
        // {11 * 8, 7 * 8 - 2, 20 * 8, 16},
        {11 * ui::new_font_width, 3* ui::new_font_height, 3 * ui::new_font_width, ui::new_font_height},
        "0/0",
        true
    };
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    Button button_clear_list{
        {1 * ui::new_font_width, screen_height - (3* ui::new_font_height), 6 * ui::new_font_width,2* ui::new_font_height},
        "Clear"};

    Button button_save_list{
<<<<<<< HEAD
        {UI_POS_X_CENTER(11), screen_height - (16 + 32), 11 * 8, 32},
        "Export CSV"};

    Button button_switch{
        {UI_POS_X_RIGHT(6), screen_height - (16 + 32), 4 * 8, 32},
=======
        {10 * ui::new_font_width, screen_height - (3* ui::new_font_height), 11 * ui::new_font_width,2* ui::new_font_height},
        "Export CSV"};

    Button button_switch{
        {23 * ui::new_font_width, screen_height - (3* ui::new_font_height), 3 * ui::new_font_width, 2*ui::new_font_height},
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb
        "Tx"};

    std::string str_log{""};
    std::unique_ptr<BLELogger> logger{};

    BleRecentEntries recent{};
    BleRecentEntries tempList{};

<<<<<<< HEAD
    RecentEntriesColumns columns{{
        {"Name", 0},
        {"Hits", 7},
        {"dBm", 4},
=======
    const RecentEntriesColumns columns{{
        {"Mac Address", 13},
        {"Hits", 7},
        {"dB", 3},
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb
    }};

    BleRecentEntriesView recent_entries_view{columns, recent};

    MessageHandlerRegistration message_handler_packet{
        Message::ID::BlePacket,
        [this](Message* const p) {
            const auto message = static_cast<const BLEPacketMessage*>(p);
            this->on_data(message->packet);
        }};

    MessageHandlerRegistration message_handler_frame_sync{
        Message::ID::DisplayFrameSync,
        [this](const Message* const) {
            this->on_timer();
        }};
}; /* BLERxView */

} /* namespace ui */

#endif /*__UI_AFSK_RX_H__*/
