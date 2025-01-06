#pragma once

#include <vector>

#include "buttons_config.hpp"
#include "leds_config.hpp"
#include "pico/stdlib.h"

class KeysConfig {
  public:
    KeysConfig(std::vector<ButtonConfig> keys_default) : keys(keys_default) {
        keys_cnt = keys_default.size();
    };
    ~KeysConfig() = default;

  private:
    std::vector<ButtonConfig> keys;
    uint keys_cnt;

  public:
    const std::vector<ButtonConfig>& get_key_cfgs() const { return keys; }

    uint get_keys_count() const { return keys_cnt; }

    void set_key_color(uint key_id, Color color) {
        if (key_id >= keys_cnt) {
            return;
        }
        keys[key_id].color = color;
    }

    void set_key_value(uint key_id, Button btn) {
        if (key_id >= keys_cnt) {
            return;
        }
        keys[key_id].value = btn;
    }

    Color get_key_color(uint key_id) const {
        if (key_id >= keys_cnt) {
            return Color::None;
        }
        return keys[key_id].color;
    }

    Button get_key_value(uint key_id) const {
        if (key_id >= keys_cnt) {
            return Key::NONE;
        }
        return keys[key_id].value;
    }
};
