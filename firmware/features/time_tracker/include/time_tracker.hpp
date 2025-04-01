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
#include <optional>
#include <unordered_map>

#include "pico/stdlib.h"

#define MICROSECONDS_IN_SECOND_COUNT 1'000'000UL
#define SECONDS_IN_HOUR_COUNT 3600UL
#define MICROSECONDS_IN_MILISECOND_COUNT 1'000UL
#define MILLISECONDS_IN_SECOND_COUNT 1'000UL
#define SECONDS_IN_MINUTE_COUNT 60UL

#define TRACING_TIMER_INTERVAL_MS 250UL
#define MAX_TIME_TRACKER_ENTRIES_COUNT 31
#define SAVE_INTERVALS_COUNT 4

#define MEDIUM_THRESHOLD_MS (30 * MILLISECONDS_IN_SECOND_COUNT)
#define LONG_THRESHOLD_MS (45 * MILLISECONDS_IN_SECOND_COUNT)

enum class TimeTrackerLog : uint {
    CURRENT_WORK_TIME_REPORT     = 0,
    CURRENT_MEETINGS_TIME_REPORT = 1,
    CURRENT_SESSION_ID           = 2,
};

constexpr uint WORK_TRACKING_KEY_ID    = 0;
constexpr uint MEETING_TRACKING_KEY_ID = 1;
constexpr uint FUNCTION_KEY_ID         = 2;

typedef struct {
    uint64_t start_time_us;
    uint64_t work_time_us;
    uint64_t meeting_time_us;
    bool tracking_work;
    bool tracking_meetings;
    DateTime_t tracking_date;
    bool medium_threshold_reached;
    bool long_threshold_reached;
} TimeTrackingEntry_t;

struct KeyColorInfo {
    Key key;
    Color color;
};

typedef struct {
    uint32_t magic;
    TimeTrackingEntry_t tracking_entries[MAX_TIME_TRACKER_ENTRIES_COUNT];
    uint active_session;
} TimeTrackerData_t;

class TimeTracker : public Feature {
  public:
    explicit TimeTracker(KeysConfig& keys_config, Storage& storage, Time& time)
    : Feature(keys_config), storage(storage), time(time) {
        initialize_key_color_map();
    }
    void handle(Buttons& buttons);

  private:
    TimeTrackerData_t data;
    Storage& storage;
    Time& time;
    uint intervals_count = 0;

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
    void tracker(const uint key, const bool is_long_press, const Buttons& buttons);
    void init();
    void deinit();
    std::string get_log(uint log_id) const;

    void factory_init();
    bool is_factory_required() const;
    bool is_date_empty(DateTime_t& date_time) const;
    void move_to_next_session();
    void initialize_new_session();
    void stop_tracking();
    void save_tracking_data() { storage.save_blob(BlobType::TIME_TRACKER_DATA, data); };
    bool is_time_to_save() const { return (intervals_count >= SAVE_INTERVALS_COUNT); }
    void increment_intervals_count() { intervals_count++; }
    void zero_intervals_count() { intervals_count = 0; }

    bool is_any_threshold_reached() const {
        const auto& entry = data.tracking_entries[data.active_session];
        return (entry.long_threshold_reached || entry.medium_threshold_reached);
    }

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
        return ((total_ms_work + total_ms_meeting) / (MILLISECONDS_IN_SECOND_COUNT * SECONDS_IN_HOUR_COUNT));
    }

    /* Key handling helpers */
    void handle_key_0_press(auto& entry, const bool is_long_press);
    void handle_key_1_press(auto& entry, const bool is_long_press);
    void handle_key_2_press(auto& entry, const bool is_long_press);

    /* Leds handling helpers */
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
        for (uint i = 0; i < keys_config.get_keys_count(); ++i) {
            led_enable(i, color);
            sleep_ms(500);
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

    static bool timer_callback(repeating_timer_t* timer);
};
