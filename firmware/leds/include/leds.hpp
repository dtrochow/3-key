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

#include <vector>

#include "buttons.hpp"
#include "hardware/pio.h"
#include "keys_config.hpp"
#include "leds_config.hpp"

class Leds {
  public:
    Leds(uint leds_count, KeysConfig& keys, PIO pio = pio0, uint pin = DEFAULT_LED_PIN, float freq = DEFAULT_FREQ);
    ~Leds() = default;

  private:
    KeysConfig& keys;

    uint leds_count;
    float w_freq;
    uint w_pin;
    uint offset;
    PIO pio;
    uint sm;
    std::vector<Led> leds;

  public:
    void init();
    void enable(uint led_id, bool r = false);
    void disable(uint led_id, bool r = false);
    void blink(uint led_id, uint count, float freq = 1);
    void blink(uint count, float freq = 1);
    void refresh() const;
    void enable_all(bool r = false);
    void disable_all(bool r = false);

  private:
    void push_led(const Led& led) const;
};

void leds_task(Leds& leds, const Buttons& buttons);
