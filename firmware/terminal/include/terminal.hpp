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

#include <cstdint>
#include <span>

#include "features_handler.hpp"
#include "keys_config.hpp"
#include "storage.hpp"

#include "binary_mode.hpp"
#include "text_mode.hpp"

class Terminal {
  private:
    bool is_binary_mode;
    TextMode text_mode;
    BinaryMode binary_mode;

  public:
    Terminal(Storage& storage, KeysConfig& keys, FeaturesHandler& f_handler);
    ~Terminal() = default;

    std::span<uint8_t> terminal(char byte);
};
