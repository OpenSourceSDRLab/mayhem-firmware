/*
 * Copyright (C) 2016 Jared Boone, ShareBrained Technology, Inc.
 * Copyright (C) 2016 Furrtek
 * Copyright (C) 2023 Kyle Reed, zxkmm
 * Copyright (C) 2024 Mark Thompson
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

#include "app_settings.hpp"
#include "bitmap.hpp"
#include "file.hpp"
#include "metadata_file.hpp"
#include "radio_state.hpp"
#include "replay_thread.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_spectrum.hpp"
#include "ui_transmitter.hpp"
#include "ui_widget.hpp"
//加上一个debug函数
#include "usb_serial_asyncmsg.hpp"

#include <string>
#include <memory>
#include <vector>

namespace ui {

class PlaylistView : public View {
   public:
    // 首次会调用这里
   PlaylistView(NavigationView& nav);

   // 测试调用的是哪个
    PlaylistView(NavigationView& nav, const std::filesystem::path& path);

    ~PlaylistView();
    bool initialized_ = false;
    // Following 2 called by 'NavigationView::update_view' after view is created.
    void focus() override;
    void set_parent_rect(Rect new_parent_rect) override;

    // 这是隐藏代码？？
    void on_hide() override;

    std::string title() const override { return "Replay"; };

   private:
    NavigationView& nav_;
    TxRadioState radio_state_{};
    app_settings::SettingsManager settings_{
        "tx_replay", app_settings::Mode::TX};

    // More header == less spectrum view.
    // static constexpr ui::Dim header_height = 6 * 16;
    
    static constexpr ui::Dim header_height = 8 * 24;

    struct playlist_entry {
        std::filesystem::path path{};
        capture_metadata metadata{};
        File::Size file_size{};
        uint32_t ms_delay{};
    };

    std::unique_ptr<ReplayThread> replay_thread_{};
    bool ready_signal_{};  // Used to signal the ReplayThread.

    size_t current_index_{0};
    bool playlist_dirty_{};
    std::vector<playlist_entry> playlist_db_{};
    std::filesystem::path playlist_path_{};
    void clear_ui(); 
    void load_file(const std::filesystem::path& path);
    Optional<playlist_entry> load_entry(std::filesystem::path&& path);
    void on_file_changed(const std::filesystem::path& path);
    void open_file(bool prompt_save = true);
    void save_file(bool show_dialogs = true);
    void add_entry(std::filesystem::path&& path);
    void delete_entry();
    void reset();
    void show_file_error(
        const std::filesystem::path& path,
        const std::string& message);

    playlist_entry* current();

    bool is_active() const;
    bool at_end() const;

    void toggle();
    void start();
    bool next_track();
    void send_current_track();
    void stop();


    void update_ui();

    /* There are called by Message handlers. */
    void on_tx_progress(uint32_t progress);
    void handle_replay_thread_done(uint32_t return_code);

    Text text_filename{
        {0 * 8, 0 * ui::new_font_height, screen_width, ui::new_font_height},
        true
    };

    FrequencyField field_frequency{
        {0 * 8, 1 * ui::new_font_height}
    };

    Text text_sample_rate{
        {10 * ui::new_font_width, 1 * ui::new_font_height, 7 * ui::new_font_width, ui::new_font_height},
        true
    };

    ProgressBar progressbar_track{
        // {ui::screen_width - 18 *8, 1 * ui::new_font_height,  18 * 8, ui::new_font_height}
        {ui::screen_width - 18 *8, 2 * ui::new_font_height,  18 * 8, 8}

        
    };

    // (-1) to overlap with progressbar_track so there's
    // only 1 pixel between them instead of 2.
    ProgressBar progressbar_transmit{
        // {ui::screen_width - 18 *8, 2 * ui::new_font_height, 18 * 8, ui::new_font_height}
        {ui::screen_width - 18 *8, 2 * ui::new_font_height+8,  18 * 8, 8}
    };

    Text text_duration{
        {0 * 8, 2 * ui::new_font_height, 5 * 8, ui::new_font_height},
        true
    };

    // TODO: delay duration field.

    TransmitterView2 tx_view{
        {11 * 8, 3 * ui::new_font_height},
        /*short_ui*/ true};

    Checkbox check_loop{
        {21 * 8, 3 * ui::new_font_height},
        4,
        "Loop",
        true,false
    };

    ImageButton button_play{
        {screen_width - 2 * 8, 3 * ui::new_font_height, 2 * 8, 1 * ui::new_font_height},
        &bitmap_play,
        Theme::getInstance()->fg_green->foreground,
        Theme::getInstance()->fg_green->background};


    Text text_track{
        {0 * 8, 4 * ui::new_font_height, screen_width, ui::new_font_height},false
    };

    NewButton button_prev{
        {2 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_arrow_left,
        Theme::getInstance()->bg_dark->background};

    NewButton button_next{
        {6 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_arrow_right,
        Theme::getInstance()->bg_dark->background};

    NewButton button_add{
        {15 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_icon_new_file,
        Theme::getInstance()->fg_orange->foreground};

    NewButton button_delete{
        {19 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_icon_delete,
        Theme::getInstance()->fg_orange->foreground};

    NewButton button_open{
        {30 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_icon_load,
        Theme::getInstance()->fg_blue->foreground};

    NewButton button_save{
        {34 * 8, 5 * ui::new_font_height, 4 * 8, 2 * ui::new_font_height},
        "",
        &bitmap_icon_save,
        Theme::getInstance()->fg_blue->foreground};


    spectrum::WaterfallView waterfall{};

    MessageHandlerRegistration message_handler_replay_thread_error{
        Message::ID::ReplayThreadDone,
        [this](const Message* p) {
            auto message = *reinterpret_cast<const ReplayThreadDoneMessage*>(p);
            handle_replay_thread_done(message.return_code);
        }};

    MessageHandlerRegistration message_handler_fifo_signal{
        Message::ID::RequestSignal,
        [this](const Message* p) {
            auto message = static_cast<const RequestSignalMessage*>(p);
            if (message->signal == RequestSignalMessage::Signal::FillRequest) {
                ready_signal_ = true;
            }
        }};

    MessageHandlerRegistration message_handler_tx_progress{
        Message::ID::TXProgress,
        [this](const Message* p) {
            auto message = *reinterpret_cast<const TXProgressMessage*>(p);
            on_tx_progress(message.progress);
        }};
};

} /* namespace ui */
