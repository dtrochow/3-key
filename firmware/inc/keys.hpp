#pragma once

#include <map>
#include <variant>
#include <vector>

#include "class/hid/hid.h"
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

using KeyOrModifier = std::variant<Key, Modifier>;

struct KeyConfig {
    uint gpio;
    KeyOrModifier value;
};

class Keys {
    public:
    explicit Keys(const std::vector<KeyConfig>& key_configs);
    ~Keys() = default;

    private:
    std::map<KeyOrModifier, uint> keys;

    public:
    void init();
    Key get_pressed_key() const;
    uint8_t get_modifier_flags() const;
};
