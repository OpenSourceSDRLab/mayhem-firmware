/*
 * Copyright (C) 2015 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2018 Furrtek
 * Copyright (C) 2023 gullradriel, Nilorea Studio Inc.
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

#ifndef _UI_RECON
#define _UI_RECON

#include "ui.hpp"
#include "receiver_model.hpp"
#include "ui_receiver.hpp"
#include "freqman.hpp"
#include "analog_audio_app.hpp"
#include "audio.hpp"
#include "ui_mictx.hpp"
#include "ui_looking_glass_app.hpp"
#include "portapack_persistent_memory.hpp"
#include "baseband_api.hpp"
#include "string_format.hpp"
#include "file.hpp"
#include "file_path.hpp"
#include "app_settings.hpp"
#include "radio_state.hpp"
#include "ui_recon_settings.hpp"
#include "ui_transmitter.hpp"
#include "replay_thread.hpp"
#include "metadata_file.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_freq_field.hpp"
#include "ui_spectrum.hpp"

#include <string>
#include <memory>


namespace ui {

enum class recon_mode : uint8_t {
    Recon,
    Scanner,
    Manual
};

class ReconView : public View {
   public:
    int FIT_OFFSET = 320/8;
    ReconView(NavigationView& nav);
    ~ReconView();

    void focus() override;

    std::string title() const override { return "Recon"; };

    // void set_parent_rect(const Rect new_parent_rect) override;

   private:
    NavigationView& nav_;

    RxRadioState rx_radio_state_{};
    TxRadioState tx_radio_state_{
        0 /* frequency */,
        1750000 /* bandwidth */,
        500000 /* sampling rate */
    };
    app_settings::SettingsManager settings_{
        "rx_tx_recon"sv, app_settings::Mode::RX_TX};
    ;

    void reload_restart_recon();
    void check_update_ranges_from_current();
    void set_loop_config(bool v);
    void clear_freqlist_for_ui_action();
    void reset_indexes();
    void update_description();
    void audio_output_start();
    bool check_sd_card();
    size_t change_mode(freqman_index_t mod_type);
    void show_max(bool refresh_display = false);
    void recon_pause();
    void recon_resume();
    void frequency_file_load();
    void on_statistics_update(const ChannelStatistics& statistics);
    void on_index_delta(int32_t v);
    void on_stepper_delta(int32_t v);
    void colorize_waits();
    void recon_redraw();
    void handle_retune();
    void handle_coded_squelch(const uint32_t value);
    void handle_remove_current_item();
    void load_persisted_settings();
    bool recon_save_freq(const std::filesystem::path& path, size_t index, bool warn_if_exists);
    // placeholder for possible void recon_start_recording();
    void recon_stop_recording(bool exiting);

    // Returns true if 'current_index' is in bounds of frequency_list.
    bool current_is_valid();
    freqman_entry& current_entry();

    // TODO: consolidate mode bools and use recon_mode.
    recon_mode mode() const {
        if (manual_mode) return recon_mode::Manual;
        if (scanner_mode) return recon_mode::Scanner;
        return recon_mode::Recon;
    }

    jammer::jammer_range_t frequency_range{false, 0, MAX_UFREQ};  // perfect for manual recon task too...
    int32_t squelch{RECON_DEF_SQUELCH};
    int32_t db{0};
    int32_t timer{0};
    int32_t wait{RECON_DEF_WAIT_DURATION};  // in msec. if > 0 wait duration after a lock, if < 0 duration is set to 'wait' unless there is no more activity
    freqman_db frequency_list{};
    int32_t current_index{0};
    bool continuous_lock{false};
    bool freqlist_cleared_for_ui_action{false};  // flag positioned by ui widgets to manage freqlist unload/load
    std::string input_file{"RECON"};
    std::string output_file{"RECON_RESULTS"};
    std::string description{"...no description..."};
    bool autosave{true};
    bool autostart{true};
    bool continuous{true};
    bool filedelete{true};
    bool load_freqs{true};
    bool load_ranges{true};
    bool load_hamradios{true};
    bool load_repeaters{true};
    bool update_ranges{true};
    bool fwd{true};
    bool recon{true};
    bool user_pause{false};
    bool auto_record_locked{false};
    bool is_recording{false};
    uint32_t recon_lock_nb_match{RECON_DEF_NB_MATCH};
    uint32_t recon_lock_duration{RECON_MIN_LOCK_DURATION};
    uint32_t recon_match_mode{RECON_MATCH_CONTINUOUS};
    bool scanner_mode{false};
    bool manual_mode{false};
    bool sd_card_mounted{false};
    int32_t stepper{0};
    int32_t index_stepper{0};
    int64_t freq{0};
    uint32_t step{0};
    freqman_index_t def_step{0};
    freqman_entry last_entry{};
    bool entry_has_changed{false};
    uint32_t freq_lock{0};
    int64_t minfreq{0};
    int64_t maxfreq{0};
    bool has_looped{false};
    int8_t status{-1};  // 0 recon , 1 locking , 2 locked
    int32_t last_timer{-1};
    int8_t last_db{-127};
    uint16_t last_nb_match{999};
    uint16_t last_freq_lock{999};
    size_t last_list_size{0};
    int8_t last_rssi_min{127};
    int8_t last_rssi_med{0};
    int8_t last_rssi_max{-127};
    int32_t last_index{-1};
    int64_t last_freq{0};
    std::string freq_file_path{};
    systime_t chrono_start{};
    systime_t chrono_end{};

    const std::filesystem::path repeat_rec_file = u"RECON_REPEAT.C16";
    const std::filesystem::path repeat_rec_meta = u"RECON_REPEAT.TXT";
    const size_t repeat_read_size{16384};
    const size_t repeat_buffer_count{3};
    int8_t repeat_cur_rep = 0;
    int64_t repeat_sample_rate = 0;
    static constexpr uint32_t repeat_bandwidth = 2500000;
    void on_repeat_tx_progress(const uint32_t progress);
    void start_repeat();
    void stop_repeat(const bool do_loop);
    bool is_repeat_active() const;
    void handle_repeat_thread_done(const uint32_t return_code);
    void repeat_file_error(const std::filesystem::path& path, const std::string& message);
    std::filesystem::path repeat_file_path{};
    std::unique_ptr<ReplayThread> replay_thread{};
    bool repeat_ready_signal{false};
    bool recon_tx{false};

    std::filesystem::path rawfile = u"/" + repeat_rec_path + u"/" + repeat_rec_file;
    std::filesystem::path rawmeta = u"/" + repeat_rec_path + u"/" + repeat_rec_meta;

    // Persisted settings.
    SettingsStore ui_settings{
        "recon"sv,
        {
            {"input_file"sv, &input_file},
            {"output_file"sv, &output_file},
            {"lock_duration"sv, &recon_lock_duration},
            {"lock_nb_match"sv, &recon_lock_nb_match},
            {"squelch_level"sv, &squelch},
            {"match_mode"sv, &recon_match_mode},
            {"match_wait"sv, &wait},
            {"range_min"sv, &frequency_range.min},
            {"range_max"sv, &frequency_range.max},
        }};

    std::unique_ptr<RecordView> record_view{};

    Labels labels{
<<<<<<< HEAD
        {{UI_POS_X(0), UI_POS_Y(0)}, "LNA:   VGA:   AMP:  ", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X_RIGHT(6), UI_POS_Y(0)}, "VOL:  ", Theme::getInstance()->fg_light->foreground},
        {{3 * 8, 8 * 16}, "START       END", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), (22 * 8)}, "                S:          ", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), (24 * 8) + 4}, "NBLCKS:x      W,L:      ,     ", Theme::getInstance()->fg_light->foreground},
        {{UI_POS_X(0), (26 * 8) + 4}, "MODE:     ,      SQUELCH:    ", Theme::getInstance()->fg_light->foreground}};
=======
        // 第1行
        {{0 * 8, 0 * 16}, "LNA:   VGA:   AMP:  VOL:     ", Theme::getInstance()->fg_light->foreground},
        // 第9行
        {{0, 8 * ui::new_font_height}, "START       END", Theme::getInstance()->fg_light->foreground},
        // 第12行
        {{0 * 8, (11 * ui::new_font_height)}, "                S:          ", Theme::getInstance()->fg_light->foreground,false},
        {{0 * 8, (12 * ui::new_font_height)}, "NBLCKS:x      W,L:      ,     ", Theme::getInstance()->fg_light->foreground,false},
        {{0 * 8, (13 * ui::new_font_height)}, "MODE:     ,        SQUELCH:    ", Theme::getInstance()->fg_light->foreground,false}
    };
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    // 第1行开始
    LNAGainField field_lna{
<<<<<<< HEAD
        {4 * 8, UI_POS_Y(0)}};

    VGAGainField field_vga{
        {11 * 8, UI_POS_Y(0)}};

    RFAmpField field_rf_amp{
        {18 * 8, UI_POS_Y(0)}};

    AudioVolumeField field_volume{
        {UI_POS_X_RIGHT(2), UI_POS_Y(0)}};
=======
        {4 * ui::new_font_width, 0 * 16}};

    VGAGainField field_vga{
        {11 * ui::new_font_width, 0 * 16}};

    RFAmpField field_rf_amp{
        {18 * ui::new_font_width, 0 * 16}};

    AudioVolumeField field_volume{
        {24 * ui::new_font_width, 0 * 16}};
    // 第1行结束
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    // 第2行开始
    Text file_name{
        // show file used
        {0, 1 * ui::new_font_height, screen_width, ui::new_font_height},true
    };

    // 第3行开始
    Text desc_cycle{
        {0, 2 * ui::new_font_height, screen_width, ui::new_font_height},false 
    };

    // 第4行开始
    RSSI rssi{
        {0 * 16, 3 * ui::new_font_height , screen_width/2, 24},
    };
    Button button_scanner_mode{
        {screen_width - 7 * ui::new_font_width, 3 * ui::new_font_height, 7 * ui::new_font_width,  ui::new_font_height+ 8 },
        "RECON"};

    // 第5行开始
    ButtonWithEncoder text_cycle{
        {0, 4 * ui::new_font_height, 4 * ui::new_font_width, ui::new_font_height},
        "",true};

    // "/XXX -XXX db" =>  12 chars max
    Text text_max{
        {4 * ui::new_font_width, 4 * ui::new_font_height, 12 * ui::new_font_width, ui::new_font_height},true
    };

    // "XX/XX" =>  5 chars max
    Text text_nb_locks{
        {16 * ui::new_font_width, 4 * ui::new_font_height, 5 * ui::new_font_width, ui::new_font_height},true
    };


    Button button_loop_config{
        {screen_width - 7 * ui::new_font_width, 5 *  ui::new_font_height, 7 * ui::new_font_width,  ui::new_font_height+ 8},
        "[LOOP]"
    };

    Text freq_stats{
        // Show frequency stats in text mode
        {0, 6 * ui::new_font_height, 21 *8, ui::new_font_height},false
    };

    // 第6行开始
    Text big_display{
        // Show frequency in text mode
        {0, 5 * ui::new_font_height , 21 * 8, ui::new_font_height},false
    };

    Button button_config{
        {screen_width - 7 * ui::new_font_width, 7 *  ui::new_font_height, 7 * ui::new_font_width,  ui::new_font_height + 8},
        "CONFIG"
    };

    // 第7行开始
   
    
    // 第8行开始
    Text text_timer{
        // Show frequency stats in text mode
        {0, 7 * ui::new_font_height, 11 * ui::new_font_width, ui::new_font_height},true
    };

    // T: Senn. 32.000k
    Text text_ctcss{
        {14 * ui::new_font_width , 7 * ui::new_font_height, 8 * ui::new_font_width, ui::new_font_height},
        "",true};

    //第10行开始
    ButtonWithEncoder button_manual_start{
<<<<<<< HEAD
        {UI_POS_X(0), 9 * 16, 11 * 8, 28},
        ""};
=======
        {0 * ui::new_font_width, 9 * ui::new_font_height, 7 * 8 * 2, 16},
        "",false,false};
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    ButtonWithEncoder button_manual_end{
        {15  * 8 , 9 * ui::new_font_height, 7 * 8 * 2, 16},
        "",false,false};

    Button button_manual_recon{
        {ui::screen_width - 7 * 8, 9 * ui::new_font_height, 7 * 8  , ui::new_font_height},
        "SEARCH",false,false};
    
    //第12行开始
    OptionsField field_recon_match_mode{
<<<<<<< HEAD
        {UI_POS_X(0), 11 * 16},
=======
        {0 * 8, 11 * ui::new_font_height},
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb
        16,  // CONTINUOUS MATCH MODE / SPARSE TIMED MATCH MODE
        {
            {"MATCH:CONTINOUS", 0},
            {"MATCH:SPARSE", 1}},false,false
        };

    OptionsField step_mode{
        {18 * 8, 11 * ui::new_font_height},
        12,
        {},false,false 
    };

    //13
    NumberField field_nblocks{
        {8 *8, 12 * ui::new_font_height},
        // {8 *8, 1000000},
        2,
        {1, 99},
        1,
        ' ',false,false 
    };

    NumberField field_wait{
        {19 * 8, 12 *ui::new_font_height},
        // {19 * 8, 1000000},
        5,
        {-RECON_MAX_LOCK_DURATION, RECON_MAX_LOCK_DURATION},
        STATS_UPDATE_INTERVAL,
        ' ',false,false 
    };

    NumberField field_lock_wait{
        {25 *8, 12 * ui::new_font_height},
        // {25 *8, 1000000},
        4,
        {STATS_UPDATE_INTERVAL, RECON_MAX_LOCK_DURATION},
        STATS_UPDATE_INTERVAL,
        ' ',false,false 
    };

    //14
    OptionsField field_mode{
        {6 * 8, 13*ui::new_font_height},
        // {6 * 8, 100000},
        4,
        {}};

    OptionsField field_bw{
        {11 * 8, 13*ui::new_font_height},
        // {6 * 8, 100000},
        6,
        {}};

    NumberField field_squelch{
        {30 * 8, 13*ui::new_font_height},
        //  {6 * 8, 100000},
        3,
        {-90, 20},
        1,
        ' ',false,false 
    };

    // 按键组
    ButtonWithEncoder button_pause{
<<<<<<< HEAD
        {0, (15 * 16) - 4, UI_POS_WIDTH(9), 28},
        "PAUSE"};

    Button button_audio_app{
        {UI_POS_X_CENTER(9), (15 * 16) - 4, UI_POS_WIDTH(9), 28},
        "AUDIO"};

    ButtonWithEncoder button_add{
        {UI_POS_X_RIGHT(9), (15 * 16) - 4, UI_POS_WIDTH(9), 28},
        "<STORE>"};
=======
        {0, ui::screen_height - 4*ui::new_font_height, 12 * 8 ,ui::new_font_height},
        "PAUSE",false,false
    };

    Button button_audio_app{
        {16 *8 , ui::screen_height - 4*ui::new_font_height, 8* 8, ui::new_font_height},
        "AUDIO",false,false
    };

    ButtonWithEncoder button_add{
        {26 *8 , ui::screen_height - 4*ui::new_font_height , 12 * 8 , ui::new_font_height},
        "<STORE>",false,false
    };

>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    Button button_dir{
        {0, ui::screen_height - 2*ui::new_font_height, 6* 8, ui::new_font_height},
        "FW>",false,false
    };

    Button button_restart{
        {8*8,  ui::screen_height - 2*ui::new_font_height, 6* 8, ui::new_font_height},
        "RST",false,false
    };

    Button button_mic_app{
<<<<<<< HEAD
        {UI_POS_X_CENTER(9), (35 * 8) - 4, UI_POS_WIDTH(9), 28},
        "MIC TX"};

    ButtonWithEncoder button_remove{
        {UI_POS_X_RIGHT(9), (35 * 8) - 4, UI_POS_WIDTH(9), 28},
        "<REMOVE>"};

    ProgressBar progressbar{
        {UI_POS_X(0), screen_height / 2 - 16, screen_width, 32}};
=======
        {16*8,  ui::screen_height - 2*ui::new_font_height, 12*8, ui::new_font_height},
        "MIC TX",false,false
    };

    ButtonWithEncoder button_remove{
        {26*8,  ui::screen_height - 2*ui::new_font_height , 12*8, ui::new_font_height},
        "<REMOVE>",false,false
    };


    ProgressBar progressbar{
        {0 * 8, screen_height-48, screen_width, 32}};
>>>>>>> a8149f33222353859a0f315bd7789e0ba82aefeb

    TransmitterView2 tx_view{
        // {11 * ui::new_font_width, 2 * ui::new_font_height},
        {11 * ui::new_font_width, 10000},
        /*short_ui*/ true};

    MessageHandlerRegistration message_handler_coded_squelch{
        Message::ID::CodedSquelch,
        [this](const Message* const p) {
            const auto message = *reinterpret_cast<const CodedSquelchMessage*>(p);
            handle_coded_squelch(message.value);
        }};

    MessageHandlerRegistration message_handler_stats{
        Message::ID::ChannelStatistics,
        [this](const Message* const p) {
            on_statistics_update(static_cast<const ChannelStatisticsMessage*>(p)->statistics);
        }};

    MessageHandlerRegistration message_handler_replay_thread_error{
        Message::ID::ReplayThreadDone,
        [this](const Message* p) {
            auto message = *reinterpret_cast<const ReplayThreadDoneMessage*>(p);
            handle_repeat_thread_done(message.return_code);
        }};

    MessageHandlerRegistration message_handler_fifo_signal{
        Message::ID::RequestSignal,
        [this](const Message* p) {
            auto message = static_cast<const RequestSignalMessage*>(p);
            if (message->signal == RequestSignalMessage::Signal::FillRequest) {
                repeat_ready_signal = true;
            }
        }};

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* p) {
            auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            on_repeat_tx_progress(message.progress);
        }};
};

} /* namespace ui */

#endif
