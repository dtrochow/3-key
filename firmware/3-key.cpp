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

#include "pico/multicore.h"
#include "pico/mutex.h"

#include "buttons.hpp"
#include "cdc.hpp"
#include "config.hpp"
#include "features_handler.hpp"
#include "hid.hpp"
#include "keys_config.hpp"
#include "leds.hpp"
#include "storage.hpp"
#include "terminal.hpp"
#include "time.hpp"
#include "tud.hpp"

Leds* g_leds       = nullptr;
Buttons* g_buttons = nullptr;

mutex_t g_mutex;

void leds_task_on_core1() {
    while (1) {
        mutex_enter_blocking(&g_mutex);
        leds_task(*g_leds, *g_buttons);
        mutex_exit(&g_mutex);
        sleep_ms(80);
    }
}

int main(void) {
    const std::vector<ButtonConfig> key_configs = {
        { 0, BUTTON_RIGHT_GPIO, Key::V, Color::Red },
        { 1, BUTTON_MIDDLE_GPIO, Key::C, Color::Green },
        { 2, BUTTON_LEFT_GPIO, Modifier::LEFT_CMD, Color::Blue },
    };

    mutex_init(&g_mutex);

    Storage storage(g_mutex);
    storage.init();

    KeysConfig keys(key_configs, storage);
    Leds leds(3, keys);
    leds.init();

    Buttons buttons(keys);
    buttons.init();

    g_buttons = &buttons;
    g_leds    = &leds;
    multicore_launch_core1(leds_task_on_core1);

    Time time;

    FeaturesHandler f_handler(storage, keys, time);
    f_handler.init();

    Terminal t(storage, keys, f_handler, time);
    CdcDevice cdc(t);

    initialize_tud();

    while (1) {
        tud_task();
        hid_task(buttons, f_handler);
        cdc.task();
    }
}
