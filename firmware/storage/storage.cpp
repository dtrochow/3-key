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

#include <cstring>
#include <limits>
#include <pico/mutex.h>

#include "storage.hpp"
#include "storage_config.hpp"

Storage::Storage(mutex_t& mutex_) : mutex(mutex_) {
    sector.resize(blobs_per_sector);
    max_blob_id        = (kStorageSize / kBlobSlotSizeBytes) - 1;
    storage_start_addr = reinterpret_cast<const uint8_t*>(XIP_BASE + kStorageFlashOffset);
}

StorageStatus_e Storage::init() {
    StorageStatus_e status = StorageStatus_e::Error;
    (void)get_blob(BlobType_e::StorageConfig, s_config);

    if (is_factory_required()) {
        status = factory_init();
        if (status != StorageStatus_e::Success)
            return status;
    }

    s_config.init_count += 1;

    return save_blob(BlobType_e::StorageConfig, s_config);
}

StorageStatus_e Storage::factory_init() {
    s_config.magic      = kBlobMagicNumber;
    s_config.init_count = 0;
    return save_blob(BlobType_e::StorageConfig, s_config);
}

uint32_t Storage::get_init_count() const {
    return s_config.init_count;
}

bool Storage::is_factory_required() {
    return (s_config.magic != kBlobMagicNumber);
}

void Storage::erase() const {
    mutex_enter_blocking(&mutex);
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(kStorageFlashOffset & ~(FLASH_SECTOR_SIZE - 1), kStorageSize);
    restore_interrupts(interrupts);
    mutex_exit(&mutex);
}

void Storage::erase(uint sector_id) const {
    const uint32_t interrupts   = save_and_disable_interrupts();
    const uintptr_t sector_addr = kStorageFlashOffset + (sector_id * FLASH_SECTOR_SIZE);
    flash_range_erase(sector_addr & ~(FLASH_SECTOR_SIZE - 1), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
}

const uint8_t* Storage::get_blob_address(uint blob_id) const {
    if (blob_id > max_blob_id) {
        return nullptr;
    }

    return (storage_start_addr + (blob_id * kBlobSlotSizeBytes));
}

StorageStatus_e Storage::_get_blob(BlobType_e blob_type, std::span<uint8_t> blob) const {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return StorageStatus_e::InvalidId;
    }

    const uint8_t* blob_addr = get_blob_address(blob_id);
    if (!blob_addr)
        return StorageStatus_e::Error;

    std::memcpy(blob.data(), blob_addr, blob.size());

    return StorageStatus_e::Success;
}

StorageStatus_e Storage::_get_blob(uint blob_id, std::span<uint8_t> blob) const {
    if (blob_id > max_blob_id) {
        return StorageStatus_e::InvalidId;
    }

    const uint8_t* blob_addr = get_blob_address(blob_id);
    if (!blob_addr)
        return StorageStatus_e::Error;

    std::memcpy(blob.data(), blob_addr, blob.size());

    return StorageStatus_e::Success;
}

uint Storage::get_sector_id(BlobType_e blob_type) const {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return std::numeric_limits<unsigned int>::max();
    }

    return (blob_id / blobs_per_sector);
}

StorageStatus_e Storage::read_sector(uint sector_id) {
    const uint start_blob_id = sector_id * blobs_per_sector;
    for (uint i = 0; i < blobs_per_sector; i++) {
        StorageStatus_e status = _get_blob(start_blob_id + i, sector[i]);
        if (status != StorageStatus_e::Success) {
            return status;
        }
    }

    return StorageStatus_e::Success;
}

StorageStatus_e Storage::update_blob_in_sector(BlobType_e blob_type, std::span<uint8_t> blob) {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return StorageStatus_e::InvalidId;
    }

    BlobBuff_t blob_copy{};
    std::copy(blob.begin(), blob.end(), blob_copy.begin());

    /* Fulfill rest of the sector with 0xFF */
    if (blob.size() < kBlobSlotSizeBytes) {
        std::memset(blob_copy.data() + blob.size(), 0xFF, kBlobSlotSizeBytes - blob.size());
    }

    sector[blob_id % blobs_per_sector] = std::move(blob_copy);

    return StorageStatus_e::Success;
}

void Storage::save_sector(uint sector_id) const {
    if (sector.size() * kBlobSlotSizeBytes != FLASH_SECTOR_SIZE) {
        return;
    }
    const uintptr_t sector_addr = kStorageFlashOffset + (sector_id * FLASH_SECTOR_SIZE);

    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(sector_addr, reinterpret_cast<const uint8_t*>(sector.data()), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
}

StorageStatus_e Storage::_save_blob(BlobType_e blob_type, std::span<uint8_t> blob) {
    const uint blob_id     = static_cast<uint>(blob_type);
    StorageStatus_e status = StorageStatus_e::Error;

    if (blob_id > max_blob_id) {
        return StorageStatus_e::InvalidId;
    }

    mutex_enter_blocking(&mutex);

    const uint sector_id = get_sector_id(blob_type);

    do {
        status = read_sector(sector_id);
        if (StorageStatus_e::Success != status) {
            break;
        }

        status = update_blob_in_sector(blob_type, blob);
        if (StorageStatus_e::Success != status) {
            break;
        }

        erase(sector_id);
        save_sector(sector_id);
    } while (0);

    mutex_exit(&mutex);

    return status;
}
