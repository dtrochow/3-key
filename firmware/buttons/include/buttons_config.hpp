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

#include <variant>

#include "class/hid/hid.h"
#include "leds_config.hpp"
#include "pico/stdlib.h"

enum Key : uint8_t {
    C    = HID_KEY_C,
    V    = HID_KEY_V,
    NONE = HID_KEY_NONE,
};

enum Modifier : uint8_t {
    LEFT_CMD  = KEYBOARD_MODIFIER_LEFTGUI,
    LEFT_CTRL = KEYBOARD_MODIFIER_LEFTCTRL,
};

using Button = std::variant<Key, Modifier>;

struct ButtonConfig {
    uint button_id;
    uint gpio;
    Button key_value;
    Color color;
    bool enabled;
};
