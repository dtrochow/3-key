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
#include <cstdint>

inline constexpr uint64_t kMicrosecondsInSecondCount      = 1'000'000UL;
inline constexpr uint64_t kSecondsInHourCount             = 3600UL;
inline constexpr uint64_t kMicrosecondsInMillisecondCount = 1'000UL;
inline constexpr uint64_t kMillisecondsInSecondCount      = 1'000UL;
inline constexpr uint64_t kSecondsInMinuteCount           = 60UL;

inline constexpr uint32_t kTracingTimerIntervalMs = 250UL;
inline constexpr uint kMaxTimeTrackerEntriesCount = 31;
inline constexpr uint kSaveIntervalsCount         = 16;

inline constexpr uint64_t kMediumThresholdMsDefault = (6 * kSecondsInHourCount * kMillisecondsInSecondCount);
inline constexpr uint64_t kLongThresholdMsDefault =
    static_cast<uint64_t>(7.5 * kSecondsInHourCount * kMillisecondsInSecondCount);

inline constexpr uint kWorkTrackingKeyId    = 0;
inline constexpr uint kMeetingTrackingKeyId = 1;
inline constexpr uint kFunctionKeyId        = 2;

using SessionId = uint;

enum class TimeTrackerLog_e : uint {
    CurrentWorkTimeReport     = 0,
    CurrentMeetingsTimeReport = 1,
    CurrentSessionId          = 2,
};

enum class TrackingType_e : uint8_t {
    WorkTracking    = 0,
    MeetingTracking = 1,
    None            = 2,
};

struct TimeTrackingEntry_t {
    uint64_t start_time_us;
    uint64_t work_time_us;
    uint64_t meeting_time_us;
    bool tracking_work;
    bool tracking_meetings;
    bool medium_threshold_reached;
    bool long_threshold_reached;
    DateTime_t tracking_date;
};

struct KeyColorInfo {
    Key_e key;
    Color_e color;
};

struct TimeTrackerData_t {
    uint32_t magic;
    TimeTrackingEntry_t tracking_entries[kMaxTimeTrackerEntriesCount];
    SessionId active_session;
    uint64_t medium_threshold_ms;
    uint64_t long_threshold_ms;
};
