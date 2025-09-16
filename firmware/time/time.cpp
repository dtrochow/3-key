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

#include "time.hpp"

#ifdef UNIT_TEST
#include "mock_time.hpp"
#else
#include "pico/time.h"
#endif

enum class Month : uint8_t {
    January   = 1,
    February  = 2,
    March     = 3,
    April     = 4,
    May       = 5,
    June      = 6,
    July      = 7,
    August    = 8,
    September = 9,
    October   = 10,
    November  = 11,
    December  = 12,
};

Time::Time() : synced_time_us(0) {}
Time::~Time() = default;

uint64_t Time::get_current_time_us() const {
    const uint64_t current_time_us = get_absolute_time();
    const uint64_t elapsed_time_us = (current_time_us - synced_device_time_us);
    return ((synced_time_us == 0) ? (0) : (synced_time_us + elapsed_time_us));
}

uint64_t Time::get_current_time_ms() const {
    return (get_current_time_us() / 1000);
}

uint64_t Time::get_current_time_s() const {
    return (get_current_time_us() / 1000000);
}

DateTime_t Time::get_current_date_and_time() const {
    static constexpr uint16_t kEpochYear           = 1970;
    static constexpr uint32_t kSecondsInMinute     = 60;
    static constexpr uint32_t kSecondsInHour       = 60 * kSecondsInMinute;
    static constexpr uint32_t kSecondsInDay        = 24 * kSecondsInHour;
    static constexpr uint32_t kSecondsInCommonYear = 365 * kSecondsInDay;
    static constexpr uint32_t kSecondsInLeapYear   = 366 * kSecondsInDay;

    static constexpr uint8_t kDaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    uint64_t total_seconds = get_current_time_s();
    uint16_t year          = kEpochYear;

    while (true) {
        const bool is_leap       = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        uint32_t seconds_in_year = is_leap ? kSecondsInLeapYear : kSecondsInCommonYear;
        if (total_seconds >= seconds_in_year) {
            total_seconds -= seconds_in_year;
            year++;
        } else {
            break;
        }
    }

    Month month = Month::January;
    while (true) {
        const bool is_leap         = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        uint8_t days_in_this_month = kDaysInMonth[static_cast<uint8_t>(month) - 1];
        if (is_leap && month == Month::February) {
            days_in_this_month++;
        }
        const uint32_t seconds_in_month = days_in_this_month * kSecondsInDay;
        if (total_seconds >= seconds_in_month) {
            total_seconds -= seconds_in_month;
            month = static_cast<Month>(static_cast<uint8_t>(month) + 1);
        } else {
            break;
        }
    }

    const auto day = static_cast<uint8_t>(total_seconds / kSecondsInDay) + 1;
    total_seconds %= kSecondsInDay;
    const auto hour = static_cast<uint8_t>(total_seconds / kSecondsInHour);
    total_seconds %= kSecondsInHour;
    const auto minute = static_cast<uint8_t>(total_seconds / kSecondsInMinute);
    const auto second = static_cast<uint8_t>(total_seconds % kSecondsInMinute);

    return { year, static_cast<uint8_t>(month), static_cast<uint8_t>(day), hour, minute, second };
}

std::string Time::get_current_date_and_time_string() const {
    DateTime_t dt = get_current_date_and_time();
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << static_cast<unsigned>(dt.day) << "." << std::setw(2)
        << static_cast<unsigned>(dt.month) << "." << std::setw(4) << dt.year << " " << std::setw(2)
        << static_cast<unsigned>(dt.hour) << ":" << std::setw(2) << static_cast<unsigned>(dt.minute)
        << ":" << std::setw(2) << static_cast<unsigned>(dt.second);
    return oss.str();
}

void Time::set_current_time_us(uint64_t time_us) {
    synced_time_us        = time_us;
    synced_device_time_us = get_absolute_time();
}
