#include <cstring>
#include <limits>

#include "storage.hpp"

Storage::Storage() {
    sector.resize(FLASH_SECTOR_SIZE / sizeof(config_t));
    max_config_id      = (STORAGE_SIZE / sizeof(config_t)) - 1;
    storage_start_addr = (const uint8_t*)(XIP_BASE + STORAGE_FLASH_OFFSET);
}

void Storage::erase() const {
    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(STORAGE_FLASH_OFFSET & ~(FLASH_SECTOR_SIZE - 1), STORAGE_SIZE);
    restore_interrupts(interrupts);
}

void Storage::erase(uint sector_id) const {
    uint32_t interrupts         = save_and_disable_interrupts();
    const uintptr_t sector_addr = STORAGE_FLASH_OFFSET + (sector_id * FLASH_SECTOR_SIZE);
    flash_range_erase(sector_addr & ~(FLASH_SECTOR_SIZE - 1), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
}

const uint8_t* Storage::get_config_address(uint config_id) const {
    if (config_id > max_config_id) {
        return nullptr;
    }

    return (storage_start_addr + (config_id * sizeof(config_t)));
}

StorageStatus Storage::get_config(uint config_id, config_t& output) const {
    if (config_id > max_config_id) {
        return StorageStatus::INVALID_ID;
    }

    memcpy(&output, get_config_address(config_id), sizeof(config_t));

    return StorageStatus::SUCCESS;
}

uint Storage::get_sector_id(uint config_id) const {
    if (config_id > max_config_id) {
        return std::numeric_limits<unsigned int>::max();
    }

    return config_id / configs_per_section;
}

StorageStatus Storage::read_sector(uint sector_id) {
    const uint start_config_id = sector_id * configs_per_section;

    for (int i = 0; i < configs_per_section; i++) {
        StorageStatus status = get_config(start_config_id + i, sector[i]);
        if (status != StorageStatus::SUCCESS) {
            return status;
        }
    }

    return StorageStatus::SUCCESS;
}

StorageStatus Storage::update_config_in_sector(uint config_id, config_t& config) {
    if (config_id > max_config_id) {
        return StorageStatus::INVALID_ID;
    }

    sector[config_id % configs_per_section] = config;

    return StorageStatus::SUCCESS;
}

void Storage::save_sector(uint sector_id) const {
    if (sector.size() * sizeof(config_t) != FLASH_SECTOR_SIZE) {
        return;
    }
    const uintptr_t sector_addr = STORAGE_FLASH_OFFSET + (sector_id * FLASH_SECTOR_SIZE);

    const uint32_t interrupts = save_and_disable_interrupts();
    flash_range_program(sector_addr, reinterpret_cast<const uint8_t*>(sector.data()), FLASH_SECTOR_SIZE);
    restore_interrupts(interrupts);
}

StorageStatus Storage::save_config(uint config_id, config_t& config) {
    StorageStatus status = StorageStatus::ERROR;

    if (config_id > max_config_id) {
        return StorageStatus::INVALID_ID;
    }

    uint sector_id = get_sector_id(config_id);

    status = read_sector(sector_id);
    if (StorageStatus::SUCCESS != status) {
        return status;
    }

    status = update_config_in_sector(config_id, config);
    if (StorageStatus::SUCCESS != status) {
        return status;
    }

    erase(sector_id);
    save_sector(sector_id);

    return StorageStatus::SUCCESS;
}
