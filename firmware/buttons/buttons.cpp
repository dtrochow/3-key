#include "buttons.hpp"
#include "buttons_config.hpp"
#include <cstdint>
#include <limits>

Buttons::Buttons(const std::vector<ButtonConfig>& key_configs) {
    for (const auto& config : key_configs) {
        buttons[config.value] = config;
    }
}

void Buttons::init() {
    for (const auto& [_, cfg] : buttons) {
        gpio_init(cfg.gpio);
        gpio_set_dir(cfg.gpio, GPIO_IN);
        gpio_pull_up(cfg.gpio);
    }
}

Key Buttons::get_pressed_key() const {
    for (const auto& [key_or_modifier, cfg] : buttons) {
        if (auto key = std::get_if<Key>(&key_or_modifier)) {
            if (!gpio_get(cfg.gpio)) {
                return *key;
            }
        }
    }
    return Key::NONE;
}

uint8_t Buttons::get_modifier_flags() const {
    uint8_t modifier_flags = 0;
    for (const auto& [key_or_modifier, cfg] : buttons) {
        if (auto modifier = std::get_if<Modifier>(&key_or_modifier)) {
            if (!gpio_get(cfg.gpio)) {
                modifier_flags |= static_cast<uint8_t>(*modifier);
            }
        }
    }
    return modifier_flags;
}

uint Buttons::get_btn_id(const Button& btn) const {
    auto it = buttons.find(btn);
    if (it != buttons.end()) {
        return it->second.id;
    }
    return std::numeric_limits<unsigned int>::max();
}

std::vector<Button> Buttons::get_btns() const {
    std::vector<Button> button_vector;
    for (const auto& [key_or_modifier, _] : buttons) {
        button_vector.push_back(key_or_modifier);
    }
    return button_vector;
}

bool Buttons::is_btn_pressed(const Button& btn) const {
    auto it = buttons.find(btn);
    if (it != buttons.end()) {
        return !gpio_get(it->second.gpio);
    }
    return false;
}

Color Buttons::get_btn_color(const Button& btn) const {
    auto it = buttons.find(btn);
    if (it != buttons.end()) {
        return it->second.color;
    }
    return Color::None;
}
