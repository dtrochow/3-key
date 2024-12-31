#pragma once

#include <vector>

#include "hardware/flash.h"
#include "hardware/sync.h"

#include "cdc.hpp"
#include "flash_config.hpp"

typedef struct {
    uint32_t magic;
    uint32_t config_value1;
    uint32_t config_value2;
    uint8_t reserved[500];
} config_t;

static_assert(sizeof(config_t) == CONFIG_SLOT_SIZE_BYTES, "Structure must have the configured size.");

enum StorageStatus {
    SUCCESS,
    INVALID_ID,
    INVALID_INPUT,
    ERROR,
};

class Storage {
  public:
    Storage(CdcDevice* cdc_dev_ = nullptr);
    ~Storage() = default;

  private:
    static constexpr uint configs_per_section = FLASH_SECTOR_SIZE / CONFIG_SLOT_SIZE_BYTES;
    std::vector<config_t> sector;
    uint32_t max_config_id;

    const uint8_t* storage_start_addr;
    const uint8_t* get_config_address(uint config_id);
    void erase();
    void erase(uint sector_id);
    uint get_sector_id(uint config_id);
    StorageStatus update_config_in_sector(uint config_id, config_t& config);
    StorageStatus read_sector(uint sector_id);
    void save_sector(uint sector_id);

    CdcDevice* cdc_dev;

    void log(const char* s);

  public:
    StorageStatus get_config(uint config_id, config_t* output);
    StorageStatus save_config(uint config_id, config_t& config);
};
