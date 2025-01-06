#pragma once

#include <map>
#include <vector>

#include "buttons_config.hpp"
#include "keys_config.hpp"

class Buttons {
  public:
    explicit Buttons(KeysConfig& keys);
    ~Buttons() = default;

  private:
    KeysConfig& keys;

  public:
    void init();
    bool is_btn_pressed(const Button& btn) const;
    Key get_pressed_key() const;
    uint8_t get_modifier_flags() const;
    uint get_btn_id(const Button& btn) const;
    std::vector<Button> get_btns() const;
};
