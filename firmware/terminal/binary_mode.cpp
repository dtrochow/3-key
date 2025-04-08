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

#include "binary_mode.hpp"
#include "features_handler.hpp"
#include "features_handler_types.hpp"
#include "time_tracker_types.hpp"
#include <cstdint>
#include <cstring>

BinaryMode::BinaryMode(Time& time_, FeaturesHandler& f_handler_)
: time(time_), binary_mode(false), f_handler(f_handler_) {}

std::span<uint8_t> BinaryMode::handle(uint8_t ch) {
    binary_buffer.push_back(ch);

    std::span<uint8_t> response = {};

    // Ensure we have at least the header and length bytes to determine packet size
    if (binary_buffer.size() >= BINARY_MODE_HEADER_SIZE_BYTES) {
        const uint32_t payload_length = static_cast<uint32_t>(binary_buffer[4]) |
            (static_cast<uint32_t>(binary_buffer[5]) << 8) |
            (static_cast<uint32_t>(binary_buffer[6]) << 16) |
            (static_cast<uint32_t>(binary_buffer[7]) << 24);

        const size_t full_packet_size = BINARY_MODE_HEADER_SIZE_BYTES + payload_length + BINARY_MODE_CRC_32_SIZE_BYTES;

        if (binary_buffer.size() == full_packet_size) {
            response = handle_binary_packet(binary_buffer);
            binary_buffer.clear();
            binary_mode = false;
        }
    }

    return response;
}

void BinaryMode::check_binary_mode(uint8_t ch) {
    if (!binary_mode && binary_buffer.empty() && (ch == BINARY_HEADER_1)) {
        binary_mode = true;
    }
}

bool BinaryMode::is_binary_mode() {
    return binary_mode;
}

BinCmdResponse BinaryMode::handle_binary_packet(const std::vector<uint8_t>& packet) {
    BinCmdResponse response = {};

    if ((packet.size() < BINARY_MODE_HEADER_SIZE_BYTES) || (packet[0] != BINARY_HEADER_1) ||
        (packet[1] != BINARY_HEADER_2)) {
        return response;
    }

    const BinaryCommandType command_type = static_cast<BinaryCommandType>(packet[2]);
    const BinaryCommandID command_id     = static_cast<BinaryCommandID>(packet[3]);

    /* Payload length field is 4 bytes long */
    const uint32_t payload_length = static_cast<uint32_t>(packet[4]) |
        (static_cast<uint32_t>(packet[5]) << 8) | (static_cast<uint32_t>(packet[6]) << 16) |
        (static_cast<uint32_t>(packet[7]) << 24);

    const size_t packet_size = packet.size();
    const size_t expected_packet_size = BINARY_MODE_HEADER_SIZE_BYTES + payload_length + BINARY_MODE_CRC_32_SIZE_BYTES;
    if (packet_size != expected_packet_size) {
        return response;
    }

    /* Extract payload and CRC32 */
    const std::vector<uint8_t> payload(packet.begin() + BINARY_MODE_HEADER_SIZE_BYTES,
        packet.end() - BINARY_MODE_CRC_32_SIZE_BYTES);
    uint32_t received_crc;
    std::memcpy(&received_crc, &packet[packet.size() - BINARY_MODE_CRC_32_SIZE_BYTES], sizeof(received_crc));

    /* Validate CRC32 */
    const uint32_t calculated_crc =
        calculate_crc32(packet.data(), packet.size() - BINARY_MODE_CRC_32_SIZE_BYTES);
    if (calculated_crc != received_crc) {
        return response;
    }

    switch (command_id) {
        case BinaryCommandID::SYNC_TIME:
            response = handle_sync_time_command(payload, command_type);
            break;
        case BinaryCommandID::GET_TIME_REPORT:
            response = handle_get_time_report_command(payload, command_type);
            break;
        case BinaryCommandID::GET_TIME_SESSION_ID:
            response = handle_get_time_session_id_command(payload, command_type);
            break;
        case BinaryCommandID::TIME_NEW_SESSION:
            response = handle_time_new_session_command(payload, command_type);
            break;
        case BinaryCommandID::UNKNOWN:
        default: break;
    }

    return response;
}

uint32_t BinaryMode::calculate_crc32(const uint8_t* data, size_t length) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; ++j) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

std::span<uint8_t> BinaryMode::create_binary_response(BinaryCommandID command_id,
    BinaryCommandStatus status,
    std::span<uint8_t> payload) {
    std::vector<uint8_t> response;

    response.push_back(BINARY_HEADER_2);
    response.push_back(BINARY_HEADER_1);

    response.push_back(static_cast<uint8_t>(status));
    response.push_back(static_cast<uint8_t>(command_id));

    const uint32_t payload_length = payload.size();
    response.push_back(static_cast<uint8_t>(payload_length & 0xFF));
    response.push_back(static_cast<uint8_t>((payload_length >> 8) & 0xFF));
    response.push_back(static_cast<uint8_t>((payload_length >> 16) & 0xFF));
    response.push_back(static_cast<uint8_t>((payload_length >> 24) & 0xFF));

    if (!payload.empty()) {
        response.insert(response.end(), payload.begin(), payload.end());
    }

    const uint32_t crc = calculate_crc32(response.data(), response.size());
    uint8_t crc_bytes[sizeof(crc)];
    std::memcpy(crc_bytes, &crc, sizeof(crc));
    response.insert(response.end(), crc_bytes, crc_bytes + sizeof(crc));

    return std::span<uint8_t>(response.data(), response.size());
}

/* -------------------------------------------------------------------------- */
/*                              Commands Handling                             */
/* -------------------------------------------------------------------------- */

BinCmdResponse BinaryMode::handle_sync_time_command(const std::vector<uint8_t>& payload, BinaryCommandType cmd_type) {
    constexpr size_t sync_time_payload_size = 8;

    if (cmd_type == BinaryCommandType::WRITE) {
        const size_t size = payload.size();
        if (size != sync_time_payload_size) {
            return create_binary_response(BinaryCommandID::SYNC_TIME, BinaryCommandStatus::INVALID_PAYLOAD);
        }

        uint64_t received_time;
        std::memcpy(&received_time, payload.data(), sizeof(received_time));
        time.set_current_time_us(received_time);
    } else {
        /* READ */
        return create_binary_response(BinaryCommandID::SYNC_TIME, BinaryCommandStatus::UNSUPPORTED_CMP_TYPE);
    }
    return create_binary_response(BinaryCommandID::SYNC_TIME, BinaryCommandStatus::SUCCESS);
}

BinCmdResponse BinaryMode::handle_get_time_report_command(const std::vector<uint8_t>& payload,
    BinaryCommandType cmd_type) {
    if (cmd_type != BinaryCommandType::READ) {
        return create_binary_response(BinaryCommandID::GET_TIME_REPORT, BinaryCommandStatus::UNSUPPORTED_CMP_TYPE);
    }

    if (payload.size() != sizeof(uint32_t)) {
        return create_binary_response(BinaryCommandID::GET_TIME_REPORT, BinaryCommandStatus::INVALID_PAYLOAD);
    }

    uint32_t session_id;
    std::memcpy(&session_id, payload.data(), sizeof(session_id));

    if ((session_id >= MAX_TIME_TRACKER_ENTRIES_COUNT) && (session_id != uint32_t(-1))) {
        return create_binary_response(BinaryCommandID::GET_TIME_REPORT, BinaryCommandStatus::INVALID_PAYLOAD);
    }

    const auto result = f_handler.get_cmd(FeatureType::TIME_TRACKER, GetTimeTrackerEntryCmd{ session_id });
    if (result.first != FeatureCmdStatus::SUCCESS) {
        return create_binary_response(BinaryCommandID::GET_TIME_REPORT, BinaryCommandStatus::ERROR);
    }

    const auto& entry = std::get<TimeTrackingEntry_t>(result.second);

    std::vector<uint8_t> response_payload(sizeof(entry));
    std::memcpy(response_payload.data(), &entry, sizeof(entry));

    return create_binary_response(BinaryCommandID::GET_TIME_REPORT, BinaryCommandStatus::SUCCESS,
        std::span<uint8_t>(response_payload));
}

BinCmdResponse BinaryMode::handle_get_time_session_id_command(const std::vector<uint8_t>& payload,
    BinaryCommandType cmd_type) {
    if (cmd_type != BinaryCommandType::READ) {
        return create_binary_response(BinaryCommandID::GET_TIME_SESSION_ID, BinaryCommandStatus::UNSUPPORTED_CMP_TYPE);
    }

    if (!payload.empty()) {
        return create_binary_response(BinaryCommandID::GET_TIME_SESSION_ID, BinaryCommandStatus::INVALID_PAYLOAD);
    }

    const auto result =
        f_handler.get_cmd(FeatureType::TIME_TRACKER, GetTimeTrackerCurrentActiveSessionIdCmd{});
    if (result.first != FeatureCmdStatus::SUCCESS) {
        return create_binary_response(BinaryCommandID::GET_TIME_SESSION_ID, BinaryCommandStatus::ERROR);
    }

    const auto& entry = std::get<SessionId>(result.second);

    std::vector<uint8_t> response_payload(sizeof(entry));
    std::memcpy(response_payload.data(), &entry, sizeof(entry));

    return create_binary_response(BinaryCommandID::GET_TIME_SESSION_ID,
        BinaryCommandStatus::SUCCESS, std::span<uint8_t>(response_payload));
}

BinCmdResponse BinaryMode::handle_time_new_session_command(const std::vector<uint8_t>& payload,
    BinaryCommandType cmd_type) {
    if (cmd_type != BinaryCommandType::WRITE) {
        return create_binary_response(BinaryCommandID::TIME_NEW_SESSION, BinaryCommandStatus::UNSUPPORTED_CMP_TYPE);
    }

    if (!payload.empty()) {
        return create_binary_response(BinaryCommandID::TIME_NEW_SESSION, BinaryCommandStatus::INVALID_PAYLOAD);
    }

    const auto status = f_handler.set_cmd(FeatureType::TIME_TRACKER, NewTimeTrackerSessionCmd{});
    if (status != FeatureCmdStatus::SUCCESS) {
        return create_binary_response(BinaryCommandID::TIME_NEW_SESSION, BinaryCommandStatus::ERROR);
    }

    return create_binary_response(BinaryCommandID::TIME_NEW_SESSION, BinaryCommandStatus::SUCCESS);
}
