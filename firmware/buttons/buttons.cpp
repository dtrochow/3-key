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
#include "keys_config.hpp"
#include <cstdint>
#include <limits>

static bool debounce_timer_callback(repeating_timer_t* timer);
static bool long_press_timer_callback(repeating_timer_t* timer);
static uint get_key_id(uint gpio);

/*     key_id, button_state  */
std::map<uint, ButtonState_t> button_map;

static KeysConfig* keys_gp = nullptr;

Buttons::Buttons(KeysConfig& keys_) : keys(keys_) {
    keys_gp = &keys;
}

void Buttons::init() {
    for (const auto& cfg : keys.get_key_cfgs()) {
        setup_button(cfg.gpio, cfg.button_id);
    }
}

Key Buttons::get_pressed_key() const {
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (auto key = std::get_if<Key>(&cfg.key_value)) {
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
            return cfg.button_id;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}

uint8_t Buttons::get_modifier_flags() const {
    uint8_t modifier_flags = 0;
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (auto modifier = std::get_if<Modifier>(&cfg.key_value)) {
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
        if (cfg.key_value == btn) {
            return cfg.button_id;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}

std::vector<Button> Buttons::get_btns() const {
    std::vector<Button> button_vector;
    for (const auto& cfg : keys.get_key_cfgs()) {
        button_vector.push_back(cfg.key_value);
    }
    return button_vector;
}

bool Buttons::is_btn_pressed(const Button& btn) const {
    for (const auto& cfg : keys.get_key_cfgs()) {
        if (cfg.key_value == btn && !gpio_get(cfg.gpio)) {
            return true;
        }
    }
    return false;
}

void Buttons::setup_button(uint gpio, uint key_id) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);

    button_map[key_id] = { .is_debouncing = false,
        .is_pending_handle                = false,
        .gpio                             = gpio,
        .key_id                           = key_id,
        .is_long_press                    = false,
        .long_press_timer                 = nullptr,
        .long_press_start_time            = 0 };
}

std::optional<ButtonState_t> Buttons::get_pending_button() {
    for (const auto& [key_id, button_state] : button_map) {
        if (button_state.is_pending_handle) {
            const ButtonState_t button_state_cpy = button_state;
            clear_pending(key_id);
            return button_state_cpy;
        }
    }
    return std::nullopt;
}

void Buttons::clear_pending(uint key_id) {
    ButtonState_t& state    = button_map[key_id];
    state.is_pending_handle = false;
    state.is_long_press     = false;
    state.is_debouncing     = false;
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
        const uint key_id    = get_key_id(gpio);
        ButtonState_t& state = button_map[key_id];

        if (!state.is_debouncing && !state.is_pending_handle) {
            state.is_debouncing = true;

            repeating_timer_t* debounce_timer = new repeating_timer_t;
            add_repeating_timer_ms(DEBOUNCE_DELAY_MS, debounce_timer_callback,
                reinterpret_cast<void*>(gpio), debounce_timer);

            gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);
        }
    }
}

static bool debounce_timer_callback(repeating_timer_t* timer) {
    constexpr uint LONG_PRESS_CHECK_DELAY_MS = 100;

    const uint gpio      = (uint)(uintptr_t)timer->user_data;
    ButtonState_t& state = button_map[get_key_id(gpio)];

    if (!gpio_get(gpio)) {
        state.long_press_timer      = new repeating_timer_t;
        state.long_press_start_time = to_ms_since_boot(get_absolute_time());
        add_repeating_timer_ms(LONG_PRESS_CHECK_DELAY_MS, long_press_timer_callback,
            reinterpret_cast<void*>(gpio), state.long_press_timer);
    } else {
        state.is_pending_handle = true;
        state.is_debouncing     = false;
        gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);
    }

    cancel_repeating_timer(timer);
    delete timer;

    return false;
}

static bool long_press_timer_callback(repeating_timer_t* timer) {
    const uint gpio      = (uint)(uintptr_t)timer->user_data;
    ButtonState_t& state = button_map[get_key_id(gpio)];

    if (!gpio_get(gpio)) {
        const uint64_t now        = to_ms_since_boot(get_absolute_time());
        const uint64_t elapsed_ms = (now - state.long_press_start_time);

        if (elapsed_ms >= keys_gp->get_long_press_delay_ms()) {
            state.is_long_press     = true;
            state.is_pending_handle = true;
            state.is_debouncing     = false;
            gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);

            cancel_repeating_timer(timer);
            delete timer;
            state.long_press_timer = nullptr;
        } else {
            /* Continue repeating */
            return true;
        }
    } else {
        state.is_pending_handle = true;
        state.is_debouncing     = false;
        gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);

        cancel_repeating_timer(timer);
        delete timer;
        state.long_press_timer = nullptr;
    }

    return false;
}
