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

#include <map>
#include <optional>
#include <pico/time.h>
#include <vector>

#include "buttons_config.hpp"
#include "keys_config.hpp"

inline constexpr uint kDebounceDelayMs = 100;

typedef struct {
    bool is_debouncing;
    bool is_pending_handle;
    uint gpio;
    uint key_id;
    bool is_long_press;
    repeating_timer_t* long_press_timer;
    uint long_press_start_time;
} ButtonState_t;

class Buttons {
  public:
    explicit Buttons(KeysConfig& keys);
    ~Buttons() = default;

    void init();
    bool is_btn_pressed(const Button& btn) const;
    Key_e get_pressed_key() const;
    uint get_pressed_key_id() const;
    uint8_t get_modifier_flags() const;
    uint get_btn_id(const Button& btn) const;
    std::vector<Button> get_btns() const;
    void setup_button(uint gpio, uint key_id);

    void clear_pending(uint key_id);
    std::optional<ButtonState_t> get_pending_button();

    void set_long_press_delay(uint delay_ms);

  private:
    KeysConfig& keys;
};
