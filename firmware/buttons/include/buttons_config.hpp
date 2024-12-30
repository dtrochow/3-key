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
    uint id;
    uint gpio;
    Button value;
    Color color;
};
