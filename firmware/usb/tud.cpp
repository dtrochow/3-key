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
