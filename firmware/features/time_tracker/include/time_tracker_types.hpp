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
#include "pico/stdlib.h"
#include "time.hpp"

#define MICROSECONDS_IN_SECOND_COUNT 1'000'000UL
#define SECONDS_IN_HOUR_COUNT 3600UL
#define MICROSECONDS_IN_MILISECOND_COUNT 1'000UL
#define MILLISECONDS_IN_SECOND_COUNT 1'000UL
#define SECONDS_IN_MINUTE_COUNT 60UL

#define TRACING_TIMER_INTERVAL_MS 250UL
#define MAX_TIME_TRACKER_ENTRIES_COUNT 31
#define SAVE_INTERVALS_COUNT 4

#define MEDIUM_THRESHOLD_MS_DEFAULT (6 * SECONDS_IN_HOUR_COUNT * MILLISECONDS_IN_SECOND_COUNT)
#define LONG_THRESHOLD_MS_DEFAULT (7.5 * SECONDS_IN_HOUR_COUNT * MILLISECONDS_IN_SECOND_COUNT)

using SessionId = uint;

enum class TimeTrackerLog : uint {
    CURRENT_WORK_TIME_REPORT     = 0,
    CURRENT_MEETINGS_TIME_REPORT = 1,
    CURRENT_SESSION_ID           = 2,
};

enum class TimeTrackerCommand : uint8_t {
    GET_TIME_TRACKER_ENTRY = 0,
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
    bool medium_threshold_reached;
    bool long_threshold_reached;
    DateTime_t tracking_date;
} TimeTrackingEntry_t;

struct KeyColorInfo {
    Key key;
    Color color;
};

typedef struct {
    uint32_t magic;
    TimeTrackingEntry_t tracking_entries[MAX_TIME_TRACKER_ENTRIES_COUNT];
    SessionId active_session;
    uint64_t medium_threshold;
    uint64_t long_threshold;
} TimeTrackerData_t;
