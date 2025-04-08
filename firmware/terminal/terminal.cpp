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

#include "terminal.hpp"
#include "binary_mode.hpp"

Terminal::Terminal(Storage& storage, KeysConfig& keys, FeaturesHandler& f_handler, Time& time)
: text_mode(storage, keys, f_handler), binary_mode(time, f_handler) {}

std::span<uint8_t> Terminal::terminal(char byte) {
    binary_mode.check_binary_mode(static_cast<uint8_t>(byte));

    if (binary_mode.is_binary_mode()) {
        return binary_mode.handle(static_cast<uint8_t>(byte));
    } else {
        return text_mode.handle(byte);
    }
}
