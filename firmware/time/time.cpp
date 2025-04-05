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

enum class Month : uint8_t {
    JANUARY   = 1,
    FEBRUARY  = 2,
    MARCH     = 3,
    APRIL     = 4,
    MAY       = 5,
    JUNE      = 6,
    JULY      = 7,
    AUGUST    = 8,
    SEPTEMBER = 9,
    OCTOBER   = 10,
    NOVEMBER  = 11,
    DECEMBER  = 12,
};

Time::Time() : current_time_us(0) {}
Time::~Time() = default;

uint64_t Time::get_current_time_us() const {
    return current_time_us;
}

uint64_t Time::get_current_time_ms() const {
    return current_time_us / 1000;
}

uint64_t Time::get_current_time_s() const {
    return current_time_us / 1000000;
}
DateTime_t Time::get_current_date_and_time() const {
    constexpr uint16_t EPOCH_YEAR             = 1970;
    constexpr uint32_t SECONDS_IN_MINUTE      = 60;
    constexpr uint32_t SECONDS_IN_HOUR        = 60 * SECONDS_IN_MINUTE;
    constexpr uint32_t SECONDS_IN_DAY         = 24 * SECONDS_IN_HOUR;
    constexpr uint32_t SECONDS_IN_COMMON_YEAR = 365 * SECONDS_IN_DAY;
    constexpr uint32_t SECONDS_IN_LEAP_YEAR   = 366 * SECONDS_IN_DAY;

    constexpr uint8_t DAYS_IN_MONTH[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    uint64_t total_seconds = get_current_time_s();
    uint16_t year          = EPOCH_YEAR;

    while (true) {
        const bool is_leap       = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        uint32_t seconds_in_year = is_leap ? SECONDS_IN_LEAP_YEAR : SECONDS_IN_COMMON_YEAR;
        if (total_seconds >= seconds_in_year) {
            total_seconds -= seconds_in_year;
            year++;
        } else {
            break;
        }
    }

    Month month = Month::JANUARY;
    while (true) {
        const bool is_leap         = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
        uint8_t days_in_this_month = DAYS_IN_MONTH[static_cast<uint8_t>(month) - 1];
        if (is_leap && month == Month::FEBRUARY) {
            days_in_this_month++;
        }
        const uint32_t seconds_in_month = days_in_this_month * SECONDS_IN_DAY;
        if (total_seconds >= seconds_in_month) {
            total_seconds -= seconds_in_month;
            month = static_cast<Month>(static_cast<uint8_t>(month) + 1);
        } else {
            break;
        }
    }

    const auto day = static_cast<uint8_t>(total_seconds / (SECONDS_IN_DAY + 1));
    total_seconds %= SECONDS_IN_DAY;
    const auto hour = static_cast<uint8_t>(total_seconds / SECONDS_IN_HOUR);
    total_seconds %= SECONDS_IN_HOUR;
    const auto minute = static_cast<uint8_t>(total_seconds / SECONDS_IN_MINUTE);
    const auto second = static_cast<uint8_t>(total_seconds % SECONDS_IN_MINUTE);

    return { year, static_cast<uint8_t>(month), day, hour, minute, second };
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
    current_time_us = time_us;
}
