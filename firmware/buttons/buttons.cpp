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

#include "buttons.hpp"
#include "buttons_config.hpp"
#include <cstdint>
#include <limits>

Buttons::Buttons(KeysConfig& keys) : keys(keys) {}

void Buttons::init() {
    for (const auto& cfg : keys.get_key_cfgs()) {
        gpio_init(cfg.gpio);
        gpio_set_dir(cfg.gpio, GPIO_IN);
        gpio_pull_up(cfg.gpio);
    }
}

Key Buttons::get_pressed_key() const {
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (auto key = std::get_if<Key>(&cfg.value)) {
            if (!gpio_get(cfg.gpio)) {
                return *key;
            }
        }
    }
    return Key::NONE;
}

uint8_t Buttons::get_modifier_flags() const {
    uint8_t modifier_flags = 0;
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (auto modifier = std::get_if<Modifier>(&cfg.value)) {
            if (!gpio_get(cfg.gpio)) {
                modifier_flags |= static_cast<uint8_t>(*modifier);
            }
        }
    }
    return modifier_flags;
}

uint Buttons::get_btn_id(const Button& btn) const {
    const auto& blobs = keys.get_key_cfgs();
    for (const auto& cfg : blobs) {
        if (cfg.value == btn) {
            return cfg.id;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}

std::vector<Button> Buttons::get_btns() const {
    std::vector<Button> button_vector;
    for (const auto& cfg : keys.get_key_cfgs()) {
        button_vector.push_back(cfg.value);
    }
    return button_vector;
}

bool Buttons::is_btn_pressed(const Button& btn) const {
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (cfg.value == btn && !gpio_get(cfg.gpio)) {
            return true;
        }
    }
    return false;
}
