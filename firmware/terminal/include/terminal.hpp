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

#include <map>
#include <string>
#include <vector>

#include "features_handler.hpp"
#include "keys_config.hpp"
#include "pico/stdlib.h"
#include "storage.hpp"

enum class Command {
    RESET,
    ERASE,
    CHANGE_COLOR,
    FEATURE,
    TIME,
    UNKNOWN,
};

class Terminal {
  private:
    static constexpr std::string start_string = "\r3-key>";
    static constexpr size_t max_chars         = 128;
    std::string buffer;
    Storage& storage;

  public:
    Terminal(Storage& storage, KeysConfig& keys, FeaturesHandler& f_handler);
    ~Terminal() = default;
    std::span<uint8_t> terminal(char new_char);

  private:
    KeysConfig& keys;
    FeaturesHandler& f_handler;

    /* Command strings mapping */
    std::map<std::string, Command> command_map = {
        { "reset", Command::RESET },
        { "erase", Command::ERASE },
        { "color", Command::CHANGE_COLOR },
        { "feature", Command::FEATURE },
        { "time", Command::TIME },
    };

    bool dispatch_command(Command command, const std::vector<std::string>& params);
    bool handle_command(const std::string& command_str);
    bool is_enter_pressed(char& ch) const;
    bool is_backspace_pressed(char& ch) const;
    bool is_new_valid_char(char& ch) const;
    bool is_valid_number(const std::string& str) const;

    void add_log(std::string log);

    void reset_to_bootloader() const;
    bool handle_change_color_command(const std::vector<std::string>& params);
    bool handle_feature_command(const std::vector<std::string>& params);
    bool handle_time_command(const std::vector<std::string>& params);
};
