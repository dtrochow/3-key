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

#include <array>
#include <span>
#include <vector>

#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/mutex.h"

#include "storage_config.hpp"

enum class StorageStatus_e {
    Success,
    InvalidId,
    InvalidInput,
    Error,
};

using BlobBuff_t = std::array<uint8_t, kBlobSlotSizeBytes>;

class Storage {
  public:
    Storage(mutex_t& mutex_);
    ~Storage() = default;
    StorageStatus_e init();
    StorageStatus_e factory_init();

    void erase() const;
    void erase(uint sector_id) const;
    uint32_t get_init_count() const;

    template <typename T> StorageStatus_e save_blob(BlobType_e blob_type, T& config) {
        static_assert(sizeof(T) <= kBlobSlotSizeBytes, "Blob size exceeds the maximum blob slot size.");
        std::span<uint8_t> blob_span(reinterpret_cast<uint8_t*>(&config), sizeof(T));

        return _save_blob(blob_type, blob_span);
    }

    template <typename T> StorageStatus_e get_blob(BlobType_e blob_type, T& config) const {
        static_assert(sizeof(T) <= kBlobSlotSizeBytes, "Blob size exceeds the maximum blob slot size.");
        std::span<uint8_t> blob_span(reinterpret_cast<uint8_t*>(&config), sizeof(T));

        return _get_blob(blob_type, blob_span);
    }

  private:
    StorageConfig_t s_config;
    static constexpr uint blobs_per_sector = FLASH_SECTOR_SIZE / kBlobSlotSizeBytes;
    std::vector<BlobBuff_t> sector;
    uint32_t max_blob_id;
    const uint8_t* storage_start_addr;
    mutex_t& mutex;

    const uint8_t* get_blob_address(uint blob_id) const;
    uint get_sector_id(BlobType_e blob_type) const;
    StorageStatus_e update_blob_in_sector(BlobType_e blob_type, std::span<uint8_t> blob);
    StorageStatus_e read_sector(uint sector_id);
    void save_sector(uint sector_id) const;
    StorageStatus_e _get_blob(uint blob_id, std::span<uint8_t> blob) const;
    StorageStatus_e _get_blob(BlobType_e blob_type, std::span<uint8_t> blob) const;
    StorageStatus_e _save_blob(BlobType_e blob_type, std::span<uint8_t> blob);

    bool is_factory_required();
};
