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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "class/hid/hid.h"
#pragma GCC diagnostic pop

#include "leds_config.hpp"
#include "pico/stdlib.h"

enum class Key_e : uint8_t {
    C       = HID_KEY_C,
    V       = HID_KEY_V,
    KeyNone = HID_KEY_NONE,
};

enum class Modifier_e : uint8_t {
    LeftCmd  = KEYBOARD_MODIFIER_LEFTGUI,
    LeftCtrl = KEYBOARD_MODIFIER_LEFTCTRL,
};

using Button = std::variant<Key_e, Modifier_e>;

struct ButtonConfig {
    uint button_id;
    uint gpio;
    Button key_value;
    Color_e color;
    bool enabled;
};
