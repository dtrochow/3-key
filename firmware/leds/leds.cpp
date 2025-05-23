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

#include <ranges>

#include "keys_config.hpp"
#include "leds.hpp"
#include "leds_config.hpp"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

Leds::Leds(uint leds_count_, KeysConfig& keys_, PIO pio_, uint pin, float freq)
: keys(keys_), leds_count(leds_count_), w_freq(freq), w_pin(pin), pio(pio_), sm(0),
  leds(leds_count_, Led(0, 0, 0)) {}

void Leds::init() {
    offset = static_cast<uint>(pio_add_program(pio, &ws2812_program));
    ws2812_program_init(pio, sm, offset, w_pin, w_freq, false);
    refresh();
    sleep_ms(10);
}

void Leds::enable(uint led_id, bool r) {
    if (led_id >= leds.size()) {
        return;
    }
    switch (keys.get_key_color(led_id)) {
        case Red: leds[led_id] = Led(255, 0, 0); break;
        case Green: leds[led_id] = Led(0, 255, 0); break;
        case Blue: leds[led_id] = Led(0, 0, 255); break;
        case Yellow: leds[led_id] = Led(255, 255, 0); break;
        case Purple: leds[led_id] = Led(255, 0, 255); break;
        case Orange: leds[led_id] = Led(255, 127, 0); break;
        case Cyan: leds[led_id] = Led(0, 255, 255); break;
        case None: leds[led_id] = Led(0, 0, 0); break;
        default: break;
    }
    if (r)
        refresh();
}

void Leds::disable(uint led_id, bool r) {
    if (led_id >= leds.size()) {
        return;
    }

    leds[led_id] = Led(0, 0, 0);
    if (r)
        refresh();
}

void Leds::enable_all(bool r) {
    for (size_t id = 0; id < leds.size(); id++) {
        enable(id);
    }
    if (r)
        refresh();
}

void Leds::disable_all(bool r) {
    for (size_t id = 0; id < leds.size(); id++) {
        disable(id);
    }
    if (r)
        refresh();
}

void Leds::push_led(const Led& led) const {
    const uint32_t color = (static_cast<uint32_t>(led.red) << 16) |
        (static_cast<uint32_t>(led.green) << 8) | (static_cast<uint32_t>(led.blue));

    pio_sm_put_blocking(pio, sm, color << 8u);
}

void Leds::refresh() const {
    for (const auto led : std::ranges::reverse_view(leds)) {
        push_led(led);
    }
}

void Leds::blink(uint count, float freq) {
    const uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (size_t i = 0; i < count; i++) {
        enable_all(true);
        sleep_ms(delay);
        disable_all(true);
        sleep_ms(delay);
    }
}

void Leds::blink(uint led_id, uint count, float freq) {
    const uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (size_t i = 0; i < count; i++) {
        enable(led_id, true);
        sleep_ms(delay);
        disable(led_id, true);
        sleep_ms(delay);
    }
}

LedsMode Leds::mode() const {
    return keys.get_leds_mode();
}

void Leds::update_led_states() {
    for (size_t i = 0; i < leds.size(); i++) {
        if (keys.is_enabled(i))
            enable(i);
        else
            disable(i);
    }
}

void leds_task(Leds& leds, const Buttons& buttons) {
    switch (leds.mode()) {
        case LedsMode::WHEN_BUTTON_PRESSED: {
            const std::vector<Button> btns = buttons.get_btns();
            for (const auto& btn : btns) {
                const uint btn_id = buttons.get_btn_id(btn);
                if (buttons.is_btn_pressed(btn)) {
                    leds.enable(btn_id);
                } else {
                    leds.disable(btn_id);
                }
            }
            break;
        }
        case LedsMode::HANDLED_BY_FEATURE: {
            leds.update_led_states();
            break;
        }
        case LedsMode::NONE:
        default: break;
    }
    leds.refresh();
}
