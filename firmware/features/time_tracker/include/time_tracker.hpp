/*
 * 3-key Project
 *
 * This file is part of the 3-key project.
 *
 * Copyright (C) 2025 Dominik Trochowski <dominik.trochowski@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "buttons.hpp"
#include "features_handler.hpp"
#include "time.hpp"
#include "time_tracker_types.hpp"
#include <optional>
#include <unordered_map>
#include <variant>
#include <vector>

#include "pico/stdlib.h"

class TimeTracker : public Feature {
  public:
    explicit TimeTracker(KeysConfig& keys_config_, Storage& storage_, Time& time_)
    : Feature(keys_config_), storage(storage_), time(time_) {
        initialize_key_color_map();
    }
    FeatureCmdResult get_cmd(const FeatureCommand& command) const override;
    FeatureCmdStatus set_cmd(const FeatureCommand& command) override;
    void handle(Buttons& buttons);

  private:
    TimeTrackerData_t data;
    Storage& storage;
    Time& time;
    uint intervals_count       = 0;
    bool awaiting_confirmation = false;
    std::vector<ButtonConfig> saved_buttons_state{};
    TrackingType previous_tracking_type = TrackingType::NONE;

    // Map to store key ID -> KeyColorInfo
    std::unordered_map<uint, KeyColorInfo> key_color_map;

    void initialize_key_color_map() {
        key_color_map[WORK_TRACKING_KEY_ID]    = { Key::NONE, Color::Purple };
        key_color_map[MEETING_TRACKING_KEY_ID] = { Key::NONE, Color::Blue };
        key_color_map[FUNCTION_KEY_ID]         = { Key::NONE, Color::Green };
    }

    std::optional<KeyColorInfo> get_key_color_info(uint key_id) const {
        auto it = key_color_map.find(key_id);
        if (it != key_color_map.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void set_tracking_date();
    void tracker(const uint key, const bool is_long_press);
    void init();
    void deinit();
    std::string get_log(uint log_id) const;

    void factory_init();
    bool is_factory_required() const;
    bool is_date_empty(DateTime_t& date_time) const;
    void move_to_next_session(bool animate = true);
    void initialize_new_session();
    void stop_tracking();
    void resume_tracking();
    void save_tracking_data() { storage.save_blob(BlobType::TIME_TRACKER_DATA, data); };
    bool is_time_to_save() const { return (intervals_count >= SAVE_INTERVALS_COUNT); }
    void increment_intervals_count() { intervals_count++; }
    void zero_intervals_count() { intervals_count = 0; }
    bool is_next_slot_empty() const;
    void save_buttons_state();
    void restore_buttons_state();

    bool is_any_threshold_reached() const {
        const auto& entry = data.tracking_entries[data.active_session];
        return (entry.long_threshold_reached || entry.medium_threshold_reached);
    }
    void check_thresholds();

    static uint64_t get_milliseconds_tracked(const TimeTrackingEntry_t& entry) {
        const uint64_t total_ms_work    = entry.work_time_us / MICROSECONDS_IN_MILISECOND_COUNT;
        const uint64_t total_ms_meeting = entry.meeting_time_us / MICROSECONDS_IN_MILISECOND_COUNT;
        return (total_ms_work + total_ms_meeting);
    }
    uint get_hours_tracked() const {
        const uint64_t total_ms_work =
            data.tracking_entries[data.active_session].work_time_us / MICROSECONDS_IN_MILISECOND_COUNT;
        const uint64_t total_ms_meeting =
            data.tracking_entries[data.active_session].meeting_time_us / MICROSECONDS_IN_MILISECOND_COUNT;
        return static_cast<uint>(
            (total_ms_work + total_ms_meeting) / (MILLISECONDS_IN_SECOND_COUNT * SECONDS_IN_HOUR_COUNT));
    }

    static bool timer_callback(repeating_timer_t* timer);

    /* -------------------------------------------------------------------------- */
    /*                            Key handling helpers                            */
    /* -------------------------------------------------------------------------- */
    void handle_key_0_press(auto& entry, const bool is_long_press);
    void handle_key_1_press(auto& entry, const bool is_long_press);
    void handle_key_2_press(const bool is_long_press);

    /* -------------------------------------------------------------------------- */
    /*                            Leds handling helpers                           */
    /* -------------------------------------------------------------------------- */
    void set_color(uint key_id, Color color) { keys_config.set_key_color(key_id, color); };
    void led_enable(uint key_id, Color color = Color::None) {
        if (color != Color::None) {
            keys_config.set_key_color(key_id, color);
        }
        keys_config.led_enable(key_id);
    };
    void led_disable(uint key_id) { keys_config.led_disable(key_id); };
    void led_toggle(uint key_id) { keys_config.led_toggle(key_id); };
    void disable_all_leds() {
        for (uint i = 0; i < keys_config.get_keys_count(); ++i) {
            keys_config.led_disable(i);
        }
    }

    void next_session_animation(Color color) {
        constexpr uint DELAY_MS = 300;
        for (uint i = 0; i < keys_config.get_keys_count(); ++i) {
            led_enable(i, color);
            sleep_ms(DELAY_MS);
        }
        disable_all_leds();
    }

    void led_blink(uint key_id, uint period, Color color = Color::None) {
        if (color != Color::None) {
            keys_config.set_key_color(key_id, color);
        }
        const uint32_t sleep_time = static_cast<uint32_t>(period / 2);
        led_enable(key_id);
        sleep_ms(sleep_time);
        led_disable(key_id);
        sleep_ms(sleep_time);
    };

    void led_blink(uint key_id, uint period, uint count, Color color = Color::None) {
        if (color != Color::None) {
            keys_config.set_key_color(key_id, color);
        }
        const uint32_t sleep_time = static_cast<uint32_t>(period / 2);
        for (uint i = 0; i < count; ++i) {
            led_enable(key_id);
            sleep_ms(sleep_time);
            led_disable(key_id);
            sleep_ms(sleep_time);
        }
    };
    /* -------------------------------------------------------------------------- */
};
