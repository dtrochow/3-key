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
#include "leds_config.hpp"
#include "pico/stdlib.h"
#include "storage.hpp"

typedef struct {
    uint id;
    Button key;
    Color color;
} KeyConfigTableEntry_t;

#define MAX_KEYS_COUNT 10

typedef struct {
    uint32_t magic;
    ButtonConfig keys[MAX_KEYS_COUNT];
    uint32_t keys_count;
} KeysConfig_t;

class KeysConfig {
  public:
    KeysConfig(std::vector<ButtonConfig> keys_default, Storage& storage) : storage(storage) {
        init(keys_default);
    }
    ~KeysConfig() = default;

    void init(const std::vector<ButtonConfig>& keys_default) {
        storage.get_blob(BlobType::KEYS_CONFIG, config);

        if (is_factory_required()) {
            factory_init(keys_default);
        }
    }

    void factory_init(const std::vector<ButtonConfig>& keys_default) {
        config.magic      = BLOB_MAGIC;
        config.keys_count = std::min(static_cast<int>(keys_default.size()), MAX_KEYS_COUNT);

        for (uint32_t i = 0; i < config.keys_count; ++i) {
            config.keys[i] = keys_default[i];
        }

        storage.save_blob(BlobType::KEYS_CONFIG, config);
    }

  private:
    KeysConfig_t config;
    Storage& storage;

    bool is_factory_required() { return (config.magic != BLOB_MAGIC); }

  public:
    std::vector<ButtonConfig> get_key_cfgs() const {
        return std::vector<ButtonConfig>(config.keys, config.keys + config.keys_count);
    }

    uint get_keys_count() const { return config.keys_count; }

    void set_key_color(uint key_id, Color color) {
        if (key_id >= config.keys_count) {
            return;
        }
        config.keys[key_id].color = color;
        storage.save_blob(BlobType::KEYS_CONFIG, config);
    }

    void set_key_value(uint key_id, Button btn) {
        if (key_id >= config.keys_count) {
            return;
        }
        config.keys[key_id].value = btn;
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
        return config.keys[key_id].value;
    }
};
