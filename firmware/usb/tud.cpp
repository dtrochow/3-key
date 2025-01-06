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

#include "tud.hpp"

// Callback function triggered when the TinyUSB device is mounted.
// Can be used to initialize or start communication.
void tud_mount_cb() {}

// Callback function triggered when the TinyUSB device is unmounted.
// Can be used to clean up resources or stop communication.
void tud_umount_cb() {}

// Callback function triggered when the TinyUSB device enters suspend state.
// The `remote_wakeup_en` parameter indicates if the host supports remote wake-up.
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}

// Callback function triggered when the TinyUSB device resumes from suspend state.
// Can be used to resume operations or reinitialize settings.
void tud_resume_cb() {}

// Function to initialize the TinyUSB device.
// Sets up the device using the TinyUSB initialization API.
void initialize_tud() {
    tud_init(BOARD_TUD_RHPORT);
}
