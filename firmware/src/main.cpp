#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsp/board_api.h"
#include "class/cdc/cdc_device.h"
#include "class/hid/hid.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "usb_descriptors.h"

#include "hardware/flash.h"
#include "hardware/sync.h"

#define FLASH_SIZE (FLASH_SECTOR_SIZE * 4) // FLASH_SECTOR_SIZE - 4096
#define FLASH_TARGET_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SIZE)
#define CONFIG_MAGIC 0xDEADBEEF

typedef struct {
    uint32_t magic;
    uint32_t config_value1;
    uint32_t config_value2;
} config_t;

#include "keys.hpp"
#include "leds.hpp"

#define BUTTON_LEFT_GPIO 14
#define BUTTON_MIDDLE_GPIO 13
#define BUTTON_RIGHT_GPIO 12

void save_config(const config_t* config);
void load_config(config_t* config);

void hid_task(const Keys& keys);
void leds_task(Leds& leds, const Keys& keys);
void cdc_task();

Leds* g_leds = nullptr;
Keys* g_keys = nullptr;

void leds_task_on_core1() {
    while (1) {
        leds_task(*g_leds, *g_keys);
        sleep_ms(50);
    }
}

int main(void) {
    const std::vector<ButtonConfig> key_configs = {
        { 0, BUTTON_RIGHT_GPIO, Key::V, Color::Red },
        { 1, BUTTON_MIDDLE_GPIO, Key::C, Color::Green },
        { 2, BUTTON_LEFT_GPIO, Modifier::LEFT_CMD, Color::Blue },
    };

    Leds leds(key_configs.size());
    leds.init();

    Keys keys(key_configs);
    keys.init();

    tud_init(BOARD_TUD_RHPORT);

    config_t config;
    load_config(&config);

    if (config.magic != CONFIG_MAGIC) {
        tud_cdc_write_str("Invalid or uninitialized config, initializing...\n");

        config.magic         = CONFIG_MAGIC;
        config.config_value1 = 42;
        config.config_value2 = 100;

        save_config(&config);
    } else {
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Loaded config: value1=%u, value2=%u\n",
        config.config_value1, config.config_value2);
        tud_cdc_write_str(buffer);
    }

    config.config_value1 += 1;
    config.config_value2 -= 1;
    save_config(&config);

    tud_cdc_write_str("Updated config saved.\n");

    g_keys = &keys;
    g_leds = &leds;
    multicore_launch_core1(leds_task_on_core1);

    while (1) {
        tud_task();
        hid_task(keys);
        cdc_task();
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Loaded config: value1=%u, value2=%u\n",
        config.config_value1, config.config_value2);
        tud_cdc_write_str(buffer);
    }
}

//--------------------------------------------------------------------+
// LEDs task
//--------------------------------------------------------------------+

void leds_task(Leds& leds, const Keys& keys) {
    const std::vector<Button> buttons = keys.get_btns();
    for (const auto& btn : buttons) {
        const uint btn_id     = keys.get_btn_id(btn);
        const Color btn_color = keys.get_btn_color(btn);
        if (keys.is_btn_pressed(btn)) {
            leds.set_led_color(btn_id, btn_color);
        } else {
            leds.set_led_color(btn_id, Color::None);
        }
    }
    leds.refresh();
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb() {
}

// Invoked when device is unmounted
void tud_umount_cb() {
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
    (void)remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb() {
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

extern "C" int _write(int fd, const void* buf, size_t count) {
    if (tud_cdc_connected()) {
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
    }
    return count;
}

static void send_hid_report(uint8_t report_id, uint8_t key, const Keys& keys) {
    if (!tud_hid_ready())
        return;

    if (report_id == REPORT_ID_KEYBOARD) {
        // use to avoid send multiple consecutive zero report for keyboard
        static bool has_keyboard_key = false;

        if (key) {
            uint8_t keycode[6]     = {};
            keycode[0]             = key;
            const uint8_t modifier = keys.get_modifier_flags();

            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, modifier, keycode);
            has_keyboard_key = true;
        } else {
            // send empty key report if previously has key pressed
            if (has_keyboard_key)
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
            has_keyboard_key = false;
        }
    }
}

void hid_task(const Keys& keys) {
    constexpr uint32_t interval_ms = 10;
    static uint32_t start_ms       = 0;

    if (board_millis() - start_ms < interval_ms)
        return;
    start_ms += interval_ms;

    const uint32_t key = keys.get_pressed_key();

    if (tud_suspended() && (key != Key::NONE)) {
        tud_remote_wakeup();
    } else {
        send_hid_report(REPORT_ID_KEYBOARD, key, keys);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)instance;
    (void)report;
    (void)len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t
tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    // @TODO - Implement
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance,
uint8_t report_id,
hid_report_type_t report_type,
uint8_t const* buffer,
uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

void cdc_task() {
    if (tud_cdc_available()) {
        uint8_t buf[64];
        uint32_t count = tud_cdc_read(buf, sizeof(buf));
        // Echo back received data
        tud_cdc_write(buf, count);
        tud_cdc_write_flush();
    }
}


void tud_cdc_line_coding_cb(__unused uint8_t itf, cdc_line_coding_t const* p_line_coding) {
    if (p_line_coding->bit_rate == 1200) {
#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u
        reset_usb_boot(1u, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK); // or 0u
    }
}

void save_config(const config_t* config) {
    uint8_t temp[FLASH_PAGE_SIZE];

    // Ensure we only write complete pages
    memcpy(temp, config, sizeof(config_t));
    memset(temp + sizeof(config_t), 0xFF, FLASH_PAGE_SIZE - sizeof(config_t));

    uint32_t interrupts = save_and_disable_interrupts();
    flash_range_erase(FLASH_TARGET_OFFSET & ~(FLASH_SECTOR_SIZE - 1), FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_TARGET_OFFSET, temp, FLASH_PAGE_SIZE);
    restore_interrupts(interrupts);
}

void load_config(config_t* config) {
    const uint8_t* flash_ptr = (const uint8_t*)(XIP_BASE + FLASH_TARGET_OFFSET);
    memcpy(config, flash_ptr, sizeof(config_t));
}
