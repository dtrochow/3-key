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

#include "hardware/flash.h"

#define BLOB_SLOTS_COUNT 16
#define BLOB_SLOT_SIZE_BYTES 2048
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
