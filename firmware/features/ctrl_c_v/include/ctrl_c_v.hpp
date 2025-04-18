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

#include "buttons.hpp"
#include "features_handler.hpp"

class CtrlCVFeature : public Feature {
  public:
    explicit CtrlCVFeature(KeysConfig& keys_config_)
    : Feature(keys_config_), has_keyboard_key(false) {}
    void handle(Buttons& buttons);

  private:
    bool has_keyboard_key;

    void send_keys(const uint8_t key, const Buttons& buttons);
    void init();
    void deinit() {};
    void factory_init() {};
    std::string get_log(uint log_id) const;
};
