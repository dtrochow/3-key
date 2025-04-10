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

#include <algorithm>
#include <vector>

#include "buttons_config.hpp"
#include "buttons_interrupt.hpp"
#include "leds_config.hpp"
#include "pico/stdlib.h"
#include "storage.hpp"

typedef struct {
    uint key_id;
    Button key;
    Color color;
} KeyConfigTableEntry_t;

constexpr uint MAX_KEYS_COUNT              = 10;
constexpr uint LONG_PRESS_DELAY_MS_DEFAULT = 800;

enum class LedsMode {
    WHEN_BUTTON_PRESSED,
    HANDLED_BY_FEATURE,
    NONE,
};

typedef struct {
    uint32_t magic;
    ButtonConfig keys[MAX_KEYS_COUNT];
    uint32_t keys_count;
} KeysConfig_t;

class KeysConfig {
  public:
    KeysConfig(std::vector<ButtonConfig> keys_default, Storage& storage_)
    : storage(storage_), leds_mode(LedsMode::WHEN_BUTTON_PRESSED) {
        init(keys_default);
    }
    ~KeysConfig() = default;

  private:
    void init(const std::vector<ButtonConfig>& keys_default) {
        storage.get_blob(BlobType::KEYS_CONFIG, config);

        if (is_factory_required()) {
            factory_init(keys_default);
        }
    }

    void factory_init(const std::vector<ButtonConfig>& keys_default) {
        config.magic      = BLOB_MAGIC;
        config.keys_count = std::min(static_cast<uint>(keys_default.size()), MAX_KEYS_COUNT);

        for (uint32_t i = 0; i < config.keys_count; ++i) {
            config.keys[i] = keys_default[i];
        }

        storage.save_blob(BlobType::KEYS_CONFIG, config);
    }

  private:
    KeysConfig_t config;
    Storage& storage;
    LedsMode leds_mode;
    uint long_press_delay_ms = LONG_PRESS_DELAY_MS_DEFAULT;

    bool is_factory_required() { return (config.magic != BLOB_MAGIC); }

  public:
    bool is_enabled(uint key_id) const { return config.keys[key_id].enabled; }
    void led_enable(uint key_id) { config.keys[key_id].enabled = true; }
    void led_disable(uint key_id) { config.keys[key_id].enabled = false; }

    void led_toggle(uint key_id) {
        if (is_enabled(key_id)) {
            led_disable(key_id);
        } else {
            led_enable(key_id);
        }
    }

    LedsMode get_leds_mode() const { return leds_mode; }

    void switch_leds_mode(LedsMode mode) {
        leds_mode = mode;
        switch (leds_mode) {
            case LedsMode::WHEN_BUTTON_PRESSED: {
                for (auto const& key : get_key_cfgs()) {
                    gpio_set_irq_enabled_with_callback(key.gpio, GPIO_IRQ_LEVEL_LOW, false, &gpio_callback);
                }
                break;
            }
            case LedsMode::HANDLED_BY_FEATURE: {
                for (auto const& key : get_key_cfgs()) {
                    gpio_set_irq_enabled_with_callback(key.gpio, GPIO_IRQ_LEVEL_LOW, true, &gpio_callback);
                }
                break;
            }
            case LedsMode::NONE:
            default: break;
        }
    }

    std::vector<ButtonConfig> get_key_cfgs() const {
        return std::vector<ButtonConfig>(config.keys, config.keys + config.keys_count);
    }

    uint get_keys_count() const { return config.keys_count; }

    void set_key_color(uint key_id, Color color, bool save = false) {
        if (key_id >= config.keys_count)
            return;
        config.keys[key_id].color = color;
        if (save)
            storage.save_blob(BlobType::KEYS_CONFIG, config);
    }

    void set_key_value(uint key_id, Button btn, bool save = false) {
        if (key_id >= config.keys_count)
            return;
        config.keys[key_id].key_value = btn;
        if (save)
            storage.save_blob(BlobType::KEYS_CONFIG, config);
    }

    Color get_key_color(uint key_id) const {
        if (key_id >= config.keys_count) {
            return Color::None;
        }
        return config.keys[key_id].color;
    }

    Button get_key_value(uint key_id) const {
        if (key_id >= config.keys_count) {
            return Key::NONE;
        }
        return config.keys[key_id].key_value;
    }

    void set_long_press_delay_ms(uint delay_ms) { long_press_delay_ms = delay_ms; }
    uint get_long_press_delay_ms() const { return long_press_delay_ms; }
};
