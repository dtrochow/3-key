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

#include "cdc.hpp"
#include "terminal.hpp"

void CdcDevice::task() const {
    while (tud_cdc_available()) {
        const char c = tud_cdc_read_char();

        const std::span<uint8_t> buffer_span = t.terminal(c);

        tud_cdc_write(buffer_span.data(), buffer_span.size());
        tud_cdc_write_flush();
    }
}

void CdcDevice::log(const char* message) const {
    tud_cdc_write_str(message);
    tud_cdc_write_str("\n\r");
    tud_cdc_write_flush();
}
