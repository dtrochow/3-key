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

#include "ctrl_c_v.hpp"
#include "keys_config.hpp"
#include "tusb.h"
#include "usb_descriptors.h"

void CtrlCVFeature::init() {
    const std::vector<KeyConfigTableEntry_t> keys = {
        { 0, Key::V, Color::Red },
        { 1, Key::C, Color::Green },
        { 2, Modifier::LEFT_CMD, Color::Blue },
    };

    for (const auto& entry : keys) {
        keys_config.set_key_color(entry.id, entry.color);
        keys_config.set_key_value(entry.id, entry.key);
    }

    keys_config.switch_leds_mode(LedsMode::WHEN_BUTTON_PRESSED);
}

void CtrlCVFeature::send_keys(const uint8_t key, const Buttons& buttons) {
    if (!tud_hid_ready())
        return;

    if (key) {
        uint8_t keycode[6]     = {};
        keycode[0]             = key;
        const uint8_t modifier = buttons.get_modifier_flags();

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
        has_keyboard_key = true;
    } else {
        if (has_keyboard_key)
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
    }
}

void CtrlCVFeature::handle(Buttons& buttons) {
    const uint8_t key = buttons.get_pressed_key();
    send_keys(key, buttons);
}

std::string CtrlCVFeature::get_log(uint log_id) const {
    return "";
}
