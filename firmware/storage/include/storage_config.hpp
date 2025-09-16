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

inline constexpr uint32_t kBlobSlotsCount    = 16;
inline constexpr uint32_t kBlobSlotSizeBytes = 2048;
inline constexpr uint32_t kBlobMagicNumber   = 0xDEADBEEF;

inline constexpr uint32_t kStorageSize = (kBlobSlotsCount * kBlobSlotSizeBytes);
static_assert((kStorageSize % FLASH_SECTOR_SIZE) == 0, "The size of the storage must be multiple of sector size.");

inline constexpr uintptr_t kStorageFlashOffset = (PICO_FLASH_SIZE_BYTES - kStorageSize);

struct StorageConfig_t {
    uint32_t magic;
    uint32_t init_count;
};

enum class BlobType_e : uint {
    StorageConfig,
    FeaturesHandlerConfig,
    KeysConfig,
    TimeTrackerData,
    BlobsCount,
};
