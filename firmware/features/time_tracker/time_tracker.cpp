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

#include <limits>

#include "keys_config.hpp"
#include "leds_config.hpp"
#include "storage_config.hpp"
#include "time.hpp"
#include "time_tracker.hpp"

repeating_timer_t* tracking_timer = nullptr;

bool TimeTracker::timer_callback(repeating_timer_t* timer) {
    auto* tracker = static_cast<TimeTracker*>(timer->user_data);
    if (!tracker)
        return false;

    const uint64_t current_time_us = time_us_64();
    constexpr uint64_t elapsed_time_us = (TRACING_TIMER_INTERVAL_MS * MICROSECONDS_IN_MILISECOND_COUNT);
    auto& entry = tracker->data.tracking_entries[tracker->data.active_session];
    if (entry.tracking_work) {
        entry.work_time_us += elapsed_time_us;
    } else if (entry.tracking_meetings) {
        entry.meeting_time_us += elapsed_time_us;
    }

    const uint64_t total_tracked_time_ms = get_milliseconds_tracked(entry);
    if (total_tracked_time_ms >= MEDIUM_THRESHOLD_MS && !entry.medium_threshold_reached) {
        tracker->led_enable(FUNCTION_KEY_ID, Color::Yellow);
        entry.medium_threshold_reached = true;
    } else if (total_tracked_time_ms >= LONG_THRESHOLD_MS && !entry.long_threshold_reached) {
        tracker->led_enable(FUNCTION_KEY_ID, Color::Red);
        entry.long_threshold_reached = true;
    }

    if (tracker->is_time_to_save()) {
        tracker->save_tracking_data();
        tracker->zero_intervals_count();
    } else {
        tracker->increment_intervals_count();
    }

    return true;
}

void TimeTracker::init() {
    storage.get_blob(BlobType::TIME_TRACKER_DATA, data);
    if (is_factory_required())
        factory_init();

    disable_all_leds();
    stop_tracking();

    keys_config.switch_leds_mode(LedsMode::HANDLED_BY_FEATURE);
    set_tracking_date();

    tracking_timer = new repeating_timer_t;
    add_repeating_timer_ms(TRACING_TIMER_INTERVAL_MS, TimeTracker::timer_callback, this, tracking_timer);
}

void TimeTracker::deinit() {
    keys_config.switch_leds_mode(LedsMode::WHEN_BUTTON_PRESSED);
    disable_all_leds();

    if (tracking_timer) {
        cancel_repeating_timer(tracking_timer);
        delete tracking_timer;
        tracking_timer = nullptr;
    }
}

void TimeTracker::factory_init() {
    data.magic = BLOB_MAGIC;

    for (auto& entry : data.tracking_entries) {
        entry.start_time_us            = 0;
        entry.work_time_us             = 0;
        entry.meeting_time_us          = 0;
        entry.tracking_work            = false;
        entry.tracking_meetings        = false;
        entry.tracking_date            = {};
        entry.medium_threshold_reached = false;
        entry.long_threshold_reached   = false;
    }

    data.active_session = 0;

    save_tracking_data();
}

bool TimeTracker::is_factory_required() const {
    return (data.magic != BLOB_MAGIC);
}

void TimeTracker::stop_tracking() {
    auto& entry = data.tracking_entries[data.active_session];
    if (entry.tracking_work) {
        entry.tracking_work = false;
        led_disable(WORK_TRACKING_KEY_ID);
    } else if (entry.tracking_meetings) {
        entry.tracking_meetings = false;
        led_disable(MEETING_TRACKING_KEY_ID);
    }
}

void TimeTracker::set_tracking_date() {
    auto& entry = data.tracking_entries[data.active_session];
    if (is_date_empty(entry.tracking_date))
        entry.tracking_date = time.get_current_date_and_time();
}

void TimeTracker::handle_key_0_press(auto& entry, const bool is_long_press) {
    if (!is_long_press) {
        if (entry.tracking_work) {
            entry.tracking_work = false;
            led_disable(WORK_TRACKING_KEY_ID);
        } else {
            if (entry.tracking_meetings) {
                entry.tracking_meetings = false;
                led_disable(MEETING_TRACKING_KEY_ID);
            }
            entry.tracking_work = true;
            Color color         = get_key_color_info(WORK_TRACKING_KEY_ID)->color;
            led_enable(WORK_TRACKING_KEY_ID, color);
        }
    }
}

void TimeTracker::handle_key_1_press(auto& entry, const bool is_long_press) {
    if (!is_long_press) {
        if (entry.tracking_meetings) {
            entry.tracking_meetings = false;
            led_disable(MEETING_TRACKING_KEY_ID);
        } else {
            if (entry.tracking_work) {
                entry.tracking_work = false;
                led_disable(WORK_TRACKING_KEY_ID);
            }
            entry.tracking_meetings = true;
            Color color             = get_key_color_info(MEETING_TRACKING_KEY_ID)->color;
            led_enable(MEETING_TRACKING_KEY_ID, color);
        }
    }
}

void TimeTracker::handle_key_2_press(auto& entry, const bool is_long_press) {
    Color color = get_key_color_info(FUNCTION_KEY_ID)->color;
    if (is_long_press) {
        stop_tracking();
        led_disable(FUNCTION_KEY_ID);

        move_to_next_session();
        initialize_new_session();

        next_session_animation(color);
    } else {
        if (!is_any_threshold_reached()) {
            /* Tracked hours indicator */
            const uint hours = get_hours_tracked();
            if (hours > 0) {
                led_blink(FUNCTION_KEY_ID, 800, hours, color);
            } else {
                led_blink(FUNCTION_KEY_ID, 800, 1, color);
            }
        }
    }
}

void TimeTracker::initialize_new_session() {
    auto& entry                    = data.tracking_entries[data.active_session];
    entry.start_time_us            = 0;
    entry.work_time_us             = 0;
    entry.meeting_time_us          = 0;
    entry.tracking_work            = false;
    entry.tracking_meetings        = false;
    entry.tracking_date            = time.get_current_date_and_time();
    entry.medium_threshold_reached = false;
    entry.long_threshold_reached   = false;
}

void TimeTracker::move_to_next_session() {
    if (data.active_session < (MAX_TIME_TRACKER_ENTRIES_COUNT - 1)) {
        data.active_session++;
    } else {
        data.active_session = 0;
    }
}

void TimeTracker::tracker(const uint key_id, const bool is_long_press, const Buttons& buttons) {
    auto& entry = data.tracking_entries[data.active_session];

    set_tracking_date();

    switch (key_id) {
        case WORK_TRACKING_KEY_ID: handle_key_0_press(entry, is_long_press); break;
        case MEETING_TRACKING_KEY_ID: handle_key_1_press(entry, is_long_press); break;
        case FUNCTION_KEY_ID: handle_key_2_press(entry, is_long_press); break;
        /* Unexpected key_id */
        default: return;
    }
}

void TimeTracker::handle(Buttons& buttons) {
    const auto pressed_key = buttons.get_pending_button();
    if (pressed_key.has_value()) {
        const ButtonState_t button_state = pressed_key.value();
        const uint key_id                = button_state.key_id;
        const bool is_long_press         = button_state.is_long_press;
        tracker(key_id, is_long_press, buttons);
    }
}

std::string TimeTracker::get_log(uint log_id) const {
    std::string log;
    auto& entry = data.tracking_entries[data.active_session];
    switch (static_cast<TimeTrackerLog>(log_id)) {
        case TimeTrackerLog::CURRENT_WORK_TIME_REPORT: {
            const uint64_t total_seconds = entry.work_time_us / MICROSECONDS_IN_SECOND_COUNT;
            const uint64_t hours         = (total_seconds / SECONDS_IN_HOUR_COUNT);
            const uint64_t minutes = ((total_seconds % SECONDS_IN_HOUR_COUNT) / SECONDS_IN_MINUTE_COUNT);
            const uint64_t seconds = (total_seconds % SECONDS_IN_MINUTE_COUNT);

            log = time.get_current_date_and_time_string() + " Work: " + std::to_string(hours) +
                "h " + std::to_string(minutes) + "min " + std::to_string(seconds) + "s";
        } break;
        case TimeTrackerLog::CURRENT_MEETINGS_TIME_REPORT: {
            const uint64_t total_seconds = entry.meeting_time_us / MICROSECONDS_IN_SECOND_COUNT;
            const uint64_t hours         = (total_seconds / SECONDS_IN_HOUR_COUNT);
            const uint64_t minutes = ((total_seconds % SECONDS_IN_HOUR_COUNT) / SECONDS_IN_MINUTE_COUNT);
            const uint64_t seconds = (total_seconds % SECONDS_IN_MINUTE_COUNT);

            log = time.get_current_date_and_time_string() + " Meetings: " + std::to_string(hours) +
                "h " + std::to_string(minutes) + "min " + std::to_string(seconds) + "s";
        } break;
        case TimeTrackerLog::CURRENT_SESSION_ID:
            log = "Current session ID: " + std::to_string(data.active_session);
            break;
        default: log = "Invalid log ID";
    }
    return log;
}

bool TimeTracker::is_date_empty(DateTime_t& date_time) const {
    return ((date_time.day == 0) && (date_time.hour == 0) && (date_time.minute == 0) &&
        (date_time.year == 0) && (date_time.month == 0) && (date_time.day == 0));
}
