#pragma once

#include "hardware/flash.h"

#define BLOB_SLOTS_COUNT 16
#define BLOB_SLOT_SIZE_BYTES 1024
#define BLOB_MAGIC 0xDEADBEEF

#define STORAGE_SIZE (BLOB_SLOTS_COUNT * BLOB_SLOT_SIZE_BYTES)
static_assert((STORAGE_SIZE % FLASH_SECTOR_SIZE) == 0, "The size of the storage must be multiple of sector size.");

#define STORAGE_FLASH_OFFSET (PICO_FLASH_SIZE_BYTES - STORAGE_SIZE)

typedef struct {
    uint32_t magic;
    uint32_t init_count;
} StorageConfig_t;

enum class BlobType : uint {
    STORAGE_CONFIG = 1,
    BLOBS_COUNT,
};
