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

#include "pico/stdlib.h"

#define MICROSECONDS_IN_SECOND_COUNT 1'000'000
#define SECONDS_IN_HOUR_COUNT 3600
#define SECONDS_IN_MINUTE_COUNT 60

#define MAX_TIME_TRACKER_ENTRIES_COUNT 31

enum class TimeTrackerLog : uint {
    CURRENT_WORK_TIME_REPORT     = 0,
    CURRENT_MEETINGS_TIME_REPORT = 1,
};

typedef struct {
    uint64_t start_time_us;
    uint64_t work_time_us;
    uint64_t meeting_time_us;
    bool tracking_work;
    bool tracking_meetings;
} TimeTrackingEntry_t;

typedef struct {
    uint32_t magic;
    TimeTrackingEntry_t tracking_entries[MAX_TIME_TRACKER_ENTRIES_COUNT];
    uint current_entry;
} TimeTrackerData_t;

class TimeTracker : public Feature {
  public:
    explicit TimeTracker(KeysConfig& keys_config, Storage& storage)
    : Feature(keys_config), storage(storage) {}
    void handle(Buttons& buttons);

  private:
    TimeTrackerData_t data;
    Storage& storage;

    void tracker(const uint key, const Buttons& buttons);
    void init();
    std::string get_log(uint log_id) const;

    void factory_init();
    bool is_factory_required();
};
