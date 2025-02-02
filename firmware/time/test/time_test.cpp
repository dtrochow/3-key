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
#include <gtest/gtest.h>

class TimeTest : public ::testing::Test {
  protected:
    Time time;
};

TEST_F(TimeTest, DefaultInitialization) {
    EXPECT_EQ(time.get_current_time_us(), 0);
    EXPECT_EQ(time.get_current_time_ms(), 0);
    EXPECT_EQ(time.get_current_time_s(), 0);
}

TEST_F(TimeTest, SetCurrentTime) {
    const uint64_t test_time_us = 1234567890123;
    time.set_current_time_us(test_time_us);
    EXPECT_EQ(time.get_current_time_us(), test_time_us);
    EXPECT_EQ(time.get_current_time_ms(), test_time_us / 1000);
    EXPECT_EQ(time.get_current_time_s(), test_time_us / 1000000);
}

TEST_F(TimeTest, GetCurrentDateAndTime) {
    /* Set time to 1st January 2022, 00:00:00 (seconds since epoch: 1640995200) */
    const uint64_t test_time_us = 1640995200ULL * 1000000;
    time.set_current_time_us(test_time_us);

    DateTime_t dt = time.get_current_date_and_time();
    EXPECT_EQ(dt.year, 2022);
    EXPECT_EQ(dt.month, 1);
    EXPECT_EQ(dt.day, 1);
    EXPECT_EQ(dt.hour, 0);
    EXPECT_EQ(dt.minute, 0);
    EXPECT_EQ(dt.second, 0);
}

TEST_F(TimeTest, GetCurrentDateAndTimeString) {
    /* Set time to 31st December 2021, 23:59:59 */
    const uint64_t test_time_us = 1640995199ULL * 1000000;
    time.set_current_time_us(test_time_us);

    std::string expected = "31.12.2021 23:59:59";
    EXPECT_EQ(time.get_current_date_and_time_string(), expected);
}

TEST_F(TimeTest, LeapYearHandling) {
    /* Set time to 29th February 2020, 12:00:00 (a leap year) */
    const uint64_t test_time_us = 1582977600ULL * 1000000;
    time.set_current_time_us(test_time_us);

    DateTime_t dt = time.get_current_date_and_time();
    EXPECT_EQ(dt.year, 2020);
    EXPECT_EQ(dt.month, 2);
    EXPECT_EQ(dt.day, 29);
    EXPECT_EQ(dt.hour, 12);
    EXPECT_EQ(dt.minute, 0);
    EXPECT_EQ(dt.second, 0);
}
