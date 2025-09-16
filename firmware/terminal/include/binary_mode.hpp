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

#include "features_handler.hpp"
#include "time.hpp"
#include <cstdint>
#include <span>
#include <vector>

inline constexpr size_t kBinaryModeHeaderSizeBytes = 8;
inline constexpr size_t kkBinaryModeCrc32SizeBytes = 4;

inline constexpr uint8_t kBinaryHeader1 = 0xAA;
inline constexpr uint8_t kBinaryHeader2 = 0xBB;

enum class BinaryCommandType_e : uint8_t {
    Write = 0x01,
    Read  = 0x02,
};

enum class BinaryCommandId_e : uint8_t {
    SyncTime               = 0x01,
    GetTimeReport          = 0x02,
    GetTimeSessionId       = 0x03,
    TimeNewSession         = 0x04,
    TimeSetMediumThreshold = 0x05,
    TimeSetLongThreshold   = 0x06,
    Unknown                = 0xFF,
};

enum class BinaryCommandStatus_e : uint8_t {
    Success            = 0x00,
    Error              = 0x01,
    InvalidPayload     = 0x02,
    UnsupportedCmdType = 0x03,
    Unknown            = 0xFF,
};

using BinCmdResponse = std::span<uint8_t>;

class BinaryMode {
  public:
    BinaryMode(Time& time, FeaturesHandler& f_handler_);
    ~BinaryMode() = default;

    std::span<uint8_t> handle(uint8_t ch);
    bool is_binary_mode();
    void check_binary_mode(uint8_t ch);

  private:
    Time& time;
    bool binary_mode;
    FeaturesHandler& f_handler;
    std::vector<uint8_t> binary_buffer;

    /* -------------------------------------------------------------------------- */
    /*                              Commands handling                             */
    /* -------------------------------------------------------------------------- */
    // clang-format off
    BinCmdResponse create_binary_response(BinaryCommandId_e command_id, BinaryCommandStatus_e status, std::span<uint8_t> payload = {});
    std::span<uint8_t> handle_binary_packet(const std::vector<uint8_t>& packet);
    uint32_t calculate_crc32(const uint8_t* data, size_t length);

    BinCmdResponse handle_sync_time_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);

    /* Feature GET commands */
    BinCmdResponse handle_get_time_report_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);
    BinCmdResponse handle_get_time_session_id_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);

    /* Feature SET commands */
    BinCmdResponse handle_set_time_new_session_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);
    BinCmdResponse handle_set_time_medium_threshold_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);
    BinCmdResponse handle_set_time_long_threshold_cmd(const std::vector<uint8_t>& payload, BinaryCommandType_e cmd_type);
    // clang-format on
    /* -------------------------------------------------------------------------- */
};
