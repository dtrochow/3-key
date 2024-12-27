#pragma once

#include <map>
#include <vector>

#include "config.hpp"

class Keys {
    public:
    explicit Keys(const std::vector<ButtonConfig>& key_configs);
    ~Keys() = default;

    private:
    std::map<Button, ButtonConfig> keys;

    public:
    void init();
    bool is_btn_pressed(const Button& btn) const;
    Key get_pressed_key() const;
    uint8_t get_modifier_flags() const;
    uint get_btn_id(const Button& btn) const;
    std::vector<Button> get_btns() const;
    Color get_btn_color(const Button& btn) const;
};
