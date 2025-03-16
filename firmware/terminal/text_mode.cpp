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

#include "text_mode.hpp"
#include "features_handler.hpp"
#include "pico/bootrom.h"
#include "time_tracker.hpp"
#include <sstream>

TextMode::TextMode(Storage& storage, KeysConfig& keys, FeaturesHandler& f_handler)
: storage(storage), keys(keys), f_handler(f_handler) {
    text_buffer = start_string;
}

void TextMode::add_log(std::string log) {
    text_buffer += "\r\n" + log;
}

std::span<uint8_t> TextMode::handle(char ch) {
    const bool is_enter = is_enter_pressed(ch);

    if (is_backspace_pressed(ch)) {
        if (text_buffer.size() > (start_string.size() / sizeof(char))) {
            text_buffer.pop_back();
        }
    } else if (is_enter) {
        const std::string command = text_buffer.substr(start_string.length());
        text_buffer.clear();
        (void)handle_command(command);
        text_buffer += "\n" + start_string;
    } else if (is_new_valid_char(ch)) {
        text_buffer += ch;
    }

    output_buffer = text_buffer;

    if (is_enter)
        text_buffer = start_string;

    return std::span<uint8_t>(reinterpret_cast<uint8_t*>(output_buffer.data()), output_buffer.size());
}

bool TextMode::is_enter_pressed(char& ch) const {
    return (ch == '\r' || ch == '\n');
}

bool TextMode::is_backspace_pressed(char& ch) const {
    return (ch == '\b' || ch == 127);
}

bool TextMode::is_new_valid_char(char& ch) const {
    return (isprint(ch) && text_buffer.size() < max_chars + 6);
}

bool TextMode::handle_command(const std::string& command_str) {
    std::istringstream iss(command_str);
    std::string command_name;
    iss >> command_name;

    std::vector<std::string> parameters;
    std::string param;
    while (iss >> param) {
        parameters.push_back(param);
    }

    const auto it     = command_map.find(command_name);
    const Command cmd = (it != command_map.end()) ? it->second : Command::UNKNOWN;
    return dispatch_command(cmd, parameters);
}

bool TextMode::dispatch_command(Command command, const std::vector<std::string>& params) {
    switch (command) {
        case Command::RESET: {
            reset_to_bootloader();
            return true;
        }
        case Command::ERASE: {
            storage.erase();
            add_log("Flash storage erased");
            return true;
        }
        case Command::CHANGE_COLOR: {
            return handle_change_color_command(params);
        }
        case Command::FEATURE: {
            return handle_feature_command(params);
        }
        case Command::TIME: {
            return handle_time_command(params);
        }
        case Command::LONG_PRESS_MS: {
            return handle_long_press_ms_command(params);
        }
        default: return false;
    }
}

bool TextMode::is_valid_number(const std::string& str) const {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

/* -------------------------------------------------------------------------- */
/*                              Commands Handling                             */
/* -------------------------------------------------------------------------- */

bool TextMode::handle_change_color_command(const std::vector<std::string>& params) {
    if (params.size() < 2) {
        add_log("Error: change_color requires 2 parameters");
        return false;
    }

    const std::string& button_id_str = params[0];
    const uint button_id             = std::stoul(button_id_str);
    if (!is_valid_number(button_id_str) || (button_id >= keys.get_keys_count())) {
        add_log("Error: Invalid button ID");
        return false;
    }

    const std::string& color_name = params[1];
    Color color;
    if (color_name == "red") {
        color = Color::Red;
    } else if (color_name == "green") {
        color = Color::Green;
    } else if (color_name == "blue") {
        color = Color::Blue;
    } else {
        add_log("Error: Invalid color");
        return false;
    }

    keys.set_key_color(button_id, color);
    add_log("Changing button " + std::to_string(button_id) + " color to " + color_name);

    return true;
}

bool TextMode::handle_feature_command(const std::vector<std::string>& params) {
    if (params.size() == 0) {
        add_log("Current feature: " + f_handler.get_current_feature_name());
        return true;
    } else if (params.size() > 1) {
        add_log("Error: Too many arguments");
        return false;
    }

    const std::string& feature_name = params[0];
    FeatureType feature;

    if (feature_name == "none") {
        feature = FeatureType::NONE;
    } else if (feature_name == "ctrl_c_v") {
        feature = FeatureType::CTRL_C_V;
    } else if (feature_name == "time-tracker") {
        feature = FeatureType::TIME_TRACKER;
    } else {
        add_log("Error: Unknown feature");
        return false;
    }

    f_handler.switch_to_feature(feature);

    add_log("Feature enabled: " + feature_name);

    return true;
}

bool TextMode::handle_time_command(const std::vector<std::string>& params) {
    if (params.size() < 1) {
        add_log("Error: 1 argument required");
        return false;
    }

    if (f_handler.get_current_feature() != FeatureType::TIME_TRACKER) {
        add_log("Time-Tracker feature is disabled");
        return false;
    }

    const std::string& param = params[0];
    std::string log;
    if (param == "work") {
        log = f_handler.get_feature_log(
            FeatureType::TIME_TRACKER, static_cast<uint>(TimeTrackerLog::CURRENT_WORK_TIME_REPORT));
    } else if (param == "meetings") {
        log = f_handler.get_feature_log(FeatureType::TIME_TRACKER,
            static_cast<uint>(TimeTrackerLog::CURRENT_MEETINGS_TIME_REPORT));
    } else if (param == "session") {
        log = f_handler.get_feature_log(
            FeatureType::TIME_TRACKER, static_cast<uint>(TimeTrackerLog::CURRENT_SESSION_ID));
    } else {
        log = "Error: Unsupported argument";
    }

    add_log(log);
    return true;
}

bool TextMode::handle_long_press_ms_command(const std::vector<std::string>& params) {
    if (params.size() != 1) {
        add_log("Error: 1 argument required");
        return false;
    }

    if (!is_valid_number(params[0])) {
        add_log("Error: Invalid argument");
        return false;
    }

    const uint long_press_ms = std::stoul(params[0]);
    keys.set_long_press_delay_ms(long_press_ms);
    add_log("Long press delay set to " + std::to_string(long_press_ms) + "ms");

    return true;
}

#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u

void TextMode::reset_to_bootloader() const {
    reset_usb_boot(1u, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
}
