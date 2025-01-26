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

#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTime_t;

class Time {
  public:
    Time();
    ~Time();

    uint64_t get_current_time_us() const;
    uint64_t get_current_time_ms() const;
    uint64_t get_current_time_s() const;
    DateTime_t get_current_date_and_time() const;
    std::string get_current_date_and_time_string() const;
    void set_current_time_us(uint64_t time_us);

  private:
    uint64_t current_time_us;
};
