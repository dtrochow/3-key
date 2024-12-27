#pragma once

#include <variant>

#include "class/hid/hid.h"
#include "pico/stdlib.h"

struct Led {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Led(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : red(r), green(g), blue(b) {
    }
};

enum Color { Red, Green, Blue, None };

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
