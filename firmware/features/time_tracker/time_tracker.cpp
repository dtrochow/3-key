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
#include "storage_config.hpp"
#include "time_tracker.hpp"

void TimeTracker::init() {
    const std::vector<KeyConfigTableEntry_t> keys = {
        /* key_id key_value color */
        { 0, Key::NONE, Color::Red },
        { 1, Key::NONE, Color::Green },
        { 2, Key::NONE, Color::Blue },
    };

    for (const auto& entry : keys) {
        keys_config.set_key_color(entry.key_id, entry.color);
        keys_config.set_key_value(entry.key_id, entry.key);
    }

    storage.get_blob(BlobType::TIME_TRACKER_DATA, data);
    if (is_factory_required())
        factory_init();

    keys_config.switch_leds_mode(LedsMode::HANDLED_BY_FEATURE);
}

void TimeTracker::factory_init() {
    data.magic = BLOB_MAGIC;

    for (auto& entry : data.tracking_entries) {
        entry.start_time_us     = 0;
        entry.work_time_us      = 0;
        entry.meeting_time_us   = 0;
        entry.tracking_work     = false;
        entry.tracking_meetings = false;
    }

    data.current_entry = 0;

    storage.save_blob(BlobType::TIME_TRACKER_DATA, data);
}

bool TimeTracker::is_factory_required() {
    return (data.magic != BLOB_MAGIC);
}

void TimeTracker::tracker(const uint key_id, const Buttons& buttons) {
    const uint64_t current_time_us = time_us_64();
    auto& entry                    = data.tracking_entries[data.current_entry];
    auto& new_entry                = data.tracking_entries[data.current_entry];

    switch (key_id) {
        case 0:
            if (entry.tracking_work) {
                entry.work_time_us += (current_time_us - entry.start_time_us);
                entry.start_time_us = 0;
                entry.tracking_work = false;
                keys_config.led_disable(0);
            } else {
                if (entry.tracking_meetings) {
                    entry.meeting_time_us += (current_time_us - entry.start_time_us);
                    entry.tracking_meetings = false;
                    keys_config.led_disable(1);
                }
                entry.start_time_us = current_time_us;
                entry.tracking_work = true;
                keys_config.led_enable(0);
            }
            break;

        case 1:
            if (entry.tracking_meetings) {
                entry.meeting_time_us += (current_time_us - entry.start_time_us);
                entry.start_time_us     = 0;
                entry.tracking_meetings = false;
                keys_config.led_disable(1);
            } else {
                if (entry.tracking_work) {
                    entry.work_time_us += (current_time_us - entry.start_time_us);
                    entry.tracking_work = false;
                    keys_config.led_disable(0);
                }
                entry.start_time_us     = current_time_us;
                entry.tracking_meetings = true;
                keys_config.led_enable(1);
            }
            break;

        case 2:
            if (entry.tracking_work) {
                entry.work_time_us += (current_time_us - entry.start_time_us);
                entry.tracking_work = false;
                keys_config.led_disable(0);
            }
            if (entry.tracking_meetings) {
                entry.meeting_time_us += (current_time_us - entry.start_time_us);
                entry.tracking_meetings = false;
                keys_config.led_disable(1);
            }

            if (data.current_entry < MAX_TIME_TRACKER_ENTRIES_COUNT - 1) {
                data.current_entry++;
            } else {
                data.current_entry = 0;
            }

            new_entry.start_time_us     = 0;
            new_entry.work_time_us      = 0;
            new_entry.meeting_time_us   = 0;
            new_entry.tracking_work     = false;
            new_entry.tracking_meetings = false;
            break;

        /* Unexpected key_id */
        default: return;
    }
    storage.save_blob(BlobType::TIME_TRACKER_DATA, data);
}

void TimeTracker::handle(Buttons& buttons) {
    const auto pressed_key = buttons.get_pending_button();
    if (pressed_key.has_value()) {
        const uint key_id = pressed_key.value();
        tracker(key_id, buttons);
    }
}

std::string TimeTracker::get_log(uint log_id) const {
    std::string log;
    auto& entry = data.tracking_entries[data.current_entry];
    switch (static_cast<TimeTrackerLog>(log_id)) {
        case TimeTrackerLog::CURRENT_WORK_TIME_REPORT: {
            const uint64_t total_seconds = entry.work_time_us / MICROSECONDS_IN_SECOND_COUNT;
            const uint64_t hours         = (total_seconds / SECONDS_IN_HOUR_COUNT);
            const uint64_t minutes = ((total_seconds % SECONDS_IN_HOUR_COUNT) / SECONDS_IN_MINUTE_COUNT);
            const uint64_t seconds = (total_seconds % SECONDS_IN_MINUTE_COUNT);

            log = "Work: " + std::to_string(hours) + "h " + std::to_string(minutes) + "min " +
                std::to_string(seconds) + "s";
        } break;
        case TimeTrackerLog::CURRENT_MEETINGS_TIME_REPORT: {
            const uint64_t total_seconds = entry.meeting_time_us / MICROSECONDS_IN_SECOND_COUNT;
            const uint64_t hours         = (total_seconds / SECONDS_IN_HOUR_COUNT);
            const uint64_t minutes = ((total_seconds % SECONDS_IN_HOUR_COUNT) / SECONDS_IN_MINUTE_COUNT);
            const uint64_t seconds = (total_seconds % SECONDS_IN_MINUTE_COUNT);

            log = "Meetings: " + std::to_string(hours) + "h " + std::to_string(minutes) + "min " +
                std::to_string(seconds) + "s";
        } break;
        default: log = "Invalid log ID";
    }
    return log;
}
