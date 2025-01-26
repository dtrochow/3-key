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

static bool debounce_timer_callback(repeating_timer_t* timer);
static uint get_key_id(uint gpio);

/*     key_id, button_state  */
std::map<uint, ButtonState_t> button_map;

Buttons::Buttons(KeysConfig& keys) : keys(keys) {}

void Buttons::init() {
    for (const auto& cfg : keys.get_key_cfgs()) {
        setup_button(cfg.gpio, cfg.id);
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

uint Buttons::get_pressed_key_id() const {
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (!gpio_get(cfg.gpio)) {
            return cfg.id;
        }
    }
    return std::numeric_limits<unsigned int>::max();
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

void Buttons::setup_button(uint gpio, uint key_id) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);

    button_map[key_id] = { false, false, gpio };
}

std::optional<uint> Buttons::get_pending_button() {
    for (const auto& [key_id, button_state] : button_map) {
        if (button_state.is_pending_handle) {
            clear_pending(key_id);
            return key_id;
        }
    }
    return std::nullopt;
}

void Buttons::clear_pending(uint key_id) {
    ButtonState_t& state    = button_map[key_id];
    state.is_pending_handle = false;
}

static uint get_key_id(uint gpio) {
    for (const auto& [key_id, button_state] : button_map) {
        if (button_state.gpio == gpio) {
            return key_id;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}

void gpio_callback(uint gpio, uint32_t events) {
    if (events & GPIO_IRQ_LEVEL_LOW) {
        uint key_id          = get_key_id(gpio);
        ButtonState_t& state = button_map[key_id];

        if (!state.is_debouncing && !state.is_pending_handle) {
            state.is_debouncing = true;

            repeating_timer_t* debounce_timer = new repeating_timer_t;
            add_repeating_timer_ms(DEBOUNCE_DELAY_MS, debounce_timer_callback, (void*)gpio, debounce_timer);

            gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
        }
    }
}

static bool debounce_timer_callback(repeating_timer_t* timer) {
    const uint gpio      = (uint)(uintptr_t)timer->user_data;
    ButtonState_t& state = button_map[get_key_id(gpio)];

    state.is_pending_handle = true;

    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
    state.is_debouncing = false;

    cancel_repeating_timer(timer);
    delete timer;

    return false;
}
