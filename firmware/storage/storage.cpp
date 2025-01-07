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

Storage::Storage(mutex_t& mutex) : mutex(mutex) {
    sector.resize(blobs_per_sector);
    max_blob_id        = (STORAGE_SIZE / BLOB_SLOT_SIZE_BYTES) - 1;
    storage_start_addr = (const uint8_t*)(XIP_BASE + STORAGE_FLASH_OFFSET);
}

StorageStatus Storage::init() {
    StorageStatus status = StorageStatus::ERROR;
    (void)get_blob(BlobType::STORAGE_CONFIG, s_config);

    if (is_factory_required()) {
        status = factory_init();
        if (status != StorageStatus::SUCCESS)
            return status;
    }

    s_config.init_count += 1;

    return save_blob(BlobType::STORAGE_CONFIG, s_config);
}

StorageStatus Storage::factory_init() {
    s_config.magic      = BLOB_MAGIC;
    s_config.init_count = 0;
    return save_blob(BlobType::STORAGE_CONFIG, s_config);
}

uint32_t Storage::get_init_count() const {
    return s_config.init_count;
}

bool Storage::is_factory_required() {
    return (s_config.magic != BLOB_MAGIC);
}

void Storage::erase() const {
    mutex_enter_blocking(&mutex);
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(STORAGE_FLASH_OFFSET & ~(FLASH_SECTOR_SIZE - 1), STORAGE_SIZE);
    restore_interrupts(interrupts);
    mutex_exit(&mutex);
}

void Storage::erase(uint sector_id) const {
    mutex_enter_blocking(&mutex);
    const uint32_t interrupts   = save_and_disable_interrupts();
    const uintptr_t sector_addr = STORAGE_FLASH_OFFSET + (sector_id * FLASH_SECTOR_SIZE);
    flash_range_erase(sector_addr & ~(FLASH_SECTOR_SIZE - 1), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
    mutex_exit(&mutex);
}

const uint8_t* Storage::get_blob_address(uint blob_id) const {
    if (blob_id > max_blob_id) {
        return nullptr;
    }

    return (storage_start_addr + (blob_id * BLOB_SLOT_SIZE_BYTES));
}

StorageStatus Storage::_get_blob(BlobType blob_type, std::span<uint8_t> blob) const {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return StorageStatus::INVALID_ID;
    }

    const uint8_t* blob_addr = get_blob_address(blob_id);
    if (!blob_addr)
        return StorageStatus::ERROR;

    std::memcpy(blob.data(), blob_addr, blob.size());

    return StorageStatus::SUCCESS;
}

StorageStatus Storage::_get_blob(uint blob_id, std::span<uint8_t> blob) const {
    if (blob_id > max_blob_id) {
        return StorageStatus::INVALID_ID;
    }

    const uint8_t* blob_addr = get_blob_address(blob_id);
    if (!blob_addr)
        return StorageStatus::ERROR;

    std::memcpy(blob.data(), blob_addr, blob.size());

    return StorageStatus::SUCCESS;
}

uint Storage::get_sector_id(BlobType blob_type) const {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return std::numeric_limits<unsigned int>::max();
    }

    return (blob_id / blobs_per_sector);
}

StorageStatus Storage::read_sector(uint sector_id) {
    const uint start_blob_id = sector_id * blobs_per_sector;
    for (int i = 0; i < blobs_per_sector; i++) {
        StorageStatus status = _get_blob(start_blob_id + i, sector[i]);
        if (status != StorageStatus::SUCCESS) {
            return status;
        }
    }

    return StorageStatus::SUCCESS;
}

StorageStatus Storage::update_blob_in_sector(BlobType blob_type, std::span<uint8_t> blob) {
    const uint blob_id = static_cast<uint>(blob_type);
    if (blob_id > max_blob_id) {
        return StorageStatus::INVALID_ID;
    }

    BlobBuff_t blob_copy{};
    std::copy(blob.begin(), blob.end(), blob_copy.begin());

    /* Fulfill rest of the sector with 0xFF */
    if (blob.size() < BLOB_SLOT_SIZE_BYTES) {
        std::memset(blob_copy.data() + blob.size(), 0xFF, BLOB_SLOT_SIZE_BYTES - blob.size());
    }

    sector[blob_id % blobs_per_sector] = std::move(blob_copy);

    return StorageStatus::SUCCESS;
}

void Storage::save_sector(uint sector_id) const {
    if (sector.size() * BLOB_SLOT_SIZE_BYTES != FLASH_SECTOR_SIZE) {
        return;
    }
    const uintptr_t sector_addr = STORAGE_FLASH_OFFSET + (sector_id * FLASH_SECTOR_SIZE);

    mutex_enter_blocking(&mutex);
    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(sector_addr, reinterpret_cast<const uint8_t*>(sector.data()), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
    mutex_exit(&mutex);
}

StorageStatus Storage::_save_blob(BlobType blob_type, std::span<uint8_t> blob) {
    const uint blob_id   = static_cast<uint>(blob_type);
    StorageStatus status = StorageStatus::ERROR;

    if (blob_id > max_blob_id) {
        return StorageStatus::INVALID_ID;
    }

    const uint sector_id = get_sector_id(blob_type);

    status = read_sector(sector_id);
    if (StorageStatus::SUCCESS != status) {
        return status;
    }

    status = update_blob_in_sector(blob_type, blob);
    if (StorageStatus::SUCCESS != status) {
        return status;
    }

    erase(sector_id);
    save_sector(sector_id);

    return StorageStatus::SUCCESS;
}
