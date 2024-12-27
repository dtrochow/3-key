#include "keys.hpp"
#include <cstdint>

Keys::Keys(const std::vector<KeyConfig>& key_configs) {
    for (const auto& config : key_configs) {
        keys[config.value] = config.gpio;
    }
}

void Keys::init() {
    for (const auto& [_, gpio] : keys) {
        gpio_init(gpio);
        gpio_set_dir(gpio, GPIO_IN);
        gpio_pull_up(gpio);
    }
}

Key Keys::get_pressed_key() const {
    for (const auto& [key_or_modifier, gpio] : keys) {
        if (auto key = std::get_if<Key>(&key_or_modifier)) {
            if (!gpio_get(gpio)) {
                return *key;
            }
        }
    }
    return Key::NONE;
}

uint8_t Keys::get_modifier_flags() const {
    uint8_t modifier_flags = 0;
    for (const auto& [key_or_modifier, gpio] : keys) {
        if (auto modifier = std::get_if<Modifier>(&key_or_modifier)) {
            if (!gpio_get(gpio)) {
                modifier_flags |= static_cast<uint8_t>(*modifier);
            }
        }
    }
    return modifier_flags;
}
