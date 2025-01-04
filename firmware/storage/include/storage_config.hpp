#pragma once

#include "hardware/flash.h"

#define CONFIG_SLOTS_COUNT 32
#define CONFIG_SLOT_SIZE_BYTES 512
#define CONFIG_MAGIC 0xDEADBEEF

#define STORAGE_SIZE (CONFIG_SLOTS_COUNT * CONFIG_SLOT_SIZE_BYTES)
static_assert((STORAGE_SIZE % FLASH_SECTOR_SIZE) == 0, "The size of the storage must be multiple of sector size.");

#define STORAGE_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - STORAGE_SIZE)
