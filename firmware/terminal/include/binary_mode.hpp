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

#include "time.hpp"
#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#define BINARY_MODE_HEADER_SIZE_BYTES 8
#define BINARY_MODE_LENGTH_FIELD_SIZE_BYTES 4
#define BINARY_MODE_CRC_32_SIZE_BYTES 4

constexpr uint8_t BINARY_HEADER_1 = 0xAA;
constexpr uint8_t BINARY_HEADER_2 = 0xBB;

enum class BinaryCommandType : uint8_t {
    WRITE = 0x01,
    READ  = 0x02,
};

enum class BinaryCommandID : uint8_t {
    SYNC_TIME   = 0x01,
    TIME_REPORT = 0x02,
    UNKNOWN     = 0xFF,
};

enum class BinaryCommandStatus : uint8_t {
    SUCCESS              = 0x00,
    ERROR                = 0x01,
    INVALID_PAYLOAD      = 0x02,
    UNSUPPORTED_CMP_TYPE = 0x03,
    UNKNOWN              = 0xFF,
};

class BinaryMode {
  public:
    BinaryMode(Time& time);
    ~BinaryMode() = default;

    std::span<uint8_t> handle(uint8_t ch);
    bool is_binary_mode();
    void check_binary_mode(uint8_t ch);

  private:
    Time& time;
    bool binary_mode;
    std::vector<uint8_t> binary_buffer;

    std::span<uint8_t> create_binary_response(BinaryCommandID command_id,
        BinaryCommandStatus status,
        std::optional<std::span<uint8_t>> payload = std::nullopt);
    std::span<uint8_t> handle_binary_packet(const std::vector<uint8_t>& packet);
    uint32_t calculate_crc32(const uint8_t* data, size_t length);

    /* Commands handling */
    std::span<uint8_t> handle_sync_time_command(const std::vector<uint8_t>& payload, BinaryCommandType cmd_type);
};
