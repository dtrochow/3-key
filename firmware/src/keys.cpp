#include "keys.hpp"
#include <cstdint>
#include <limits>

Keys::Keys(const std::vector<ButtonConfig>& key_configs) {
    for (const auto& config : key_configs) {
        keys[config.value] = config;
    }
}

void Keys::init() {
    for (const auto& [_, cfg] : keys) {
        gpio_init(cfg.gpio);
        gpio_set_dir(cfg.gpio, GPIO_IN);
        gpio_pull_up(cfg.gpio);
    }
}

Key Keys::get_pressed_key() const {
    for (const auto& [key_or_modifier, cfg] : keys) {
        if (auto key = std::get_if<Key>(&key_or_modifier)) {
            if (!gpio_get(cfg.gpio)) {
                return *key;
            }
        }
    }
    return Key::NONE;
}

uint8_t Keys::get_modifier_flags() const {
    uint8_t modifier_flags = 0;
    for (const auto& [key_or_modifier, cfg] : keys) {
        if (auto modifier = std::get_if<Modifier>(&key_or_modifier)) {
            if (!gpio_get(cfg.gpio)) {
                modifier_flags |= static_cast<uint8_t>(*modifier);
            }
        }
    }
    return modifier_flags;
}

uint Keys::get_btn_id(const Button& btn) const {
    auto it = keys.find(btn);
    if (it != keys.end()) {
        return it->second.id;
    }
    return std::numeric_limits<unsigned int>::max();
}

std::vector<Button> Keys::get_btns() const {
    std::vector<Button> button_vector;
    for (const auto& [key_or_modifier, _] : keys) {
        button_vector.push_back(key_or_modifier);
    }
    return button_vector;
}

bool Keys::is_btn_pressed(const Button& btn) const {
    auto it = keys.find(btn);
    if (it != keys.end()) {
        return !gpio_get(it->second.gpio);
    }
    return false;
}

Color Keys::get_btn_color(const Button& btn) const {
    auto it = keys.find(btn);
    if (it != keys.end()) {
        return it->second.color;
    }
    return Color::None;
}
