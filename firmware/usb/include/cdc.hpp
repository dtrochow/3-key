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

#include "class/cdc/cdc_device.h"
#include "pico/stdlib.h"

#include "terminal.hpp"

void cdc_task();

class CdcDevice {
  public:
    explicit CdcDevice(Terminal& t_) : t(t_) {};
    ~CdcDevice() = default;

    void task() const;
    void log(const char* message) const;

  private:
    Terminal& t;
};
