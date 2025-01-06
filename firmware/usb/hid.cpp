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

#include "bsp/board_api.h"

#include "buttons.hpp"
#include "buttons_config.hpp"
#include "usb_descriptors.h"

static void send_hid_report(uint8_t report_id, uint8_t key, const Buttons& buttons) {
    if (!tud_hid_ready())
        return;

    if (report_id == REPORT_ID_KEYBOARD) {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_keyboard_key = false;

        if (key) {
            uint8_t keycode[6]     = {};
            keycode[0]             = key;
            const uint8_t modifier = buttons.get_modifier_flags();

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
            has_keyboard_key = true;
        } else {
            // send empty key report if previously has key pressed
            if (has_keyboard_key)
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            has_keyboard_key = false;
        }
    }
}

void hid_task(const Buttons& buttons) {
    constexpr uint32_t interval_ms = 10;
    static uint32_t start_ms       = 0;

    if (board_millis() - start_ms < interval_ms)
        return;
    start_ms += interval_ms;

    const uint32_t key = buttons.get_pressed_key();

    if (tud_suspended() && (key != Key::NONE)) {
        tud_remote_wakeup();
    } else {
        send_hid_report(REPORT_ID_KEYBOARD, key, buttons);
    }
}

void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)instance;
    (void)report;
    (void)len;
}

uint16_t
tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
    uint8_t report_id,
    hid_report_type_t report_type,
    uint8_t const* buffer,
    uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
