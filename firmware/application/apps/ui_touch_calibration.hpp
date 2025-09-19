/*
 * Copyright (C) 2016 Jared Boone, ShareBrained Technology, Inc.
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

#ifndef __UI_TOUCH_CALIBRATION_HPP__
#define __UI_TOUCH_CALIBRATION_HPP__

#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "touch.hpp"
#include "usb_serial_asyncmsg.hpp"

namespace ui {

class TouchCalibrationView : public View {
   public:
    TouchCalibrationView(NavigationView& nav);
    ~TouchCalibrationView();

    void focus() override;

    std::string title() const override { return "Calibration"; };

   private:
    enum class Phase {
        Init,
        Calibrate0,
        Calibrate1,
        Calibrate2,
        Verify0,
        Verify1,
        Verify2,
        Success,
        Failure,
    };

    NavigationView& nav;
    Phase phase{Phase::Init};

    void update_target();

    void set_phase(const Phase value);

    uint32_t distance_squared(const Point& touch_point, const Image& target);

    void touch_complete();
    void on_ok();
    void on_cancel();

    const uint32_t samples_limit{40};
    const uint32_t verify_d_sq_max = 10 * 10;

    uint32_t samples_count{0};

    touch::DigitizerPoint average{};

    std::array<touch::DigitizerPoint, 3> digitizer_points{};

    touch::Calibration calibration;

    Image image_calibrate_0{
        // {32 - 16, 32 - 16, 32, 32},
        {32, 48, 32, 32},
        &bitmap_target_calibrate,
        Color::white(),
        Color::black()};

    Image image_calibrate_1{
        // {240 - 32 - 16, (320 - 16) / 2 - 16, 32, 32},
        {208, 168, 32, 32},
        &bitmap_target_calibrate,
        Color::white(),
        Color::black()};

    Image image_calibrate_2{
        // {240 / 2 - 16, (320 - 16) - 32 - 16, 32, 32},
        {120, 288, 32, 32},
        &bitmap_target_calibrate,
        Color::white(),
        Color::black()};

    Image image_verify_0{
        // {32 - 16, 32 - 16, 32, 32},
        {32, 48, 32, 32},
        &bitmap_target_verify,
        Color::white(),
        Color::black()};

    Image image_verify_1{
        // {240 - 32 - 16, (320 - 16) / 2 - 16, 32, 32},
        {208, 168, 32, 32},
        &bitmap_target_verify,
        Color::white(),
        Color::black()};

    Image image_verify_2{
        // {240 / 2 - 16, (320 - 16) - 32 - 16, 32, 32},
        {120, 288, 32, 32},
        &bitmap_target_verify,
        Color::white(),
        Color::black()};

    Text label_calibrate{
        {2 * 8, 5 * 16, 26 * 8, 1 * 16},
        "Touch targets to calibrate",false
    };

    Text label_calibrate_2{
        {1 * 8, 6 * 16, 28 * 8, 1 * 16},
        "(hold position using stylus)",false
    };

    Text label_verify{
        {28, 5 * 16, 23 * 8, 1 * 16},
        "Touch targets to verify",false
    };

    Text label_success{
        {32, 5 * 16, 22 * 8, 1 * 16},
        "Apply new calibration?",false
    };

    Text label_failure{
        {16, 5 * 16, 26 * 8, 1 * 16},
        "Calibration failed. Retry?",false
    };

    Button button_cancel{
        {40, ui::screen_height - ui::new_font_height *3, 8*ui::new_font_width, ui::new_font_height *2},
        "Cancel"};

    Button button_ok{
        {136, ui::screen_height - ui::new_font_height *3, 8*ui::new_font_width, ui::new_font_height *2},
        "OK"};

    Text debug_text{
        {0,ui::screen_height/4*3,ui::screen_width,ui::new_font_height},
        "test string",
        true
    };

    Button button_for_debug{
        {0,ui::screen_height-ui::new_font_height *4, 8*ui::new_font_width, ui::new_font_height *2},
        "debug"
    };

    void on_frame_sync();

    // 这是什么
    MessageHandlerRegistration message_handler_frame_sync{
        Message::ID::DisplayFrameSync,
        [this](const Message* const) {
            this->on_frame_sync();
        }};
};

} /* namespace ui */

#endif /*__UI_TOUCH_CALIBRATION_HPP__*/
