#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "usb_descriptors.h"

#define LED_PIN 18      // GPIO Pin for WS2812 data
#define NUM_LEDS 3      // Number of LEDs in the strip
#define FREQ 800000     // WS2812 frequency (800kHz)

#define BUTTON_A_GPIO 12
#define BUTTON_B_GPIO 13
#define BUTTON_C_GPIO 14

void init_gpio_pins() {
  gpio_init(BUTTON_A_GPIO);
  gpio_set_dir(BUTTON_A_GPIO, GPIO_IN);
  gpio_pull_up(BUTTON_A_GPIO);

  gpio_init(BUTTON_B_GPIO);
  gpio_set_dir(BUTTON_B_GPIO, GPIO_IN);
  gpio_pull_up(BUTTON_B_GPIO);

  gpio_init(BUTTON_C_GPIO);
  gpio_set_dir(BUTTON_C_GPIO, GPIO_IN);
  gpio_pull_up(BUTTON_C_GPIO);
}

void hid_task(void);

void set_led_color(PIO pio, uint sm, uint32_t color) {
    pio_sm_put_blocking(pio, sm, color << 8u);
}

PIO pio = pio0;
uint sm = 0;

int main(void)
{
  uint offset = pio_add_program(pio, &ws2812_program);
  ws2812_program_init(pio, sm, offset, LED_PIN, FREQ, false);

  uint32_t red = 0x00FF0000;
  for (int i = 0; i < NUM_LEDS; i++) {
      set_led_color(pio, sm, red);
  }
  sleep_ms(3000);

  init_gpio_pins();
  tud_init(BOARD_TUD_RHPORT);

  while (1) {
    tud_task(); // tinyusb device task
    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  uint32_t green = 0x0000FF00;
  for (int i = 0; i < 5; i ++) {
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_color(pio, sm, green);
    }
    sleep_ms(500);
    for (int i = 0; i < NUM_LEDS; i++) {
        set_led_color(pio, sm, 0x0);
    }
    sleep_ms(500);
  }
}

// Invoked when device is unmounted
void tud_umount_cb(void) {}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  if ( !tud_hid_ready() ) return;

  if ( report_id == REPORT_ID_KEYBOARD ) {
    // use to avoid send multiple consecutive zero report for keyboard
    static bool has_keyboard_key = false;

    if ( btn ) {
      uint8_t keycode[6] = { 0 };
      keycode[0] = 0x15;

      tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
      has_keyboard_key = true;
    } else {
      // send empty key report if previously has key pressed
      if ( has_keyboard_key ) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
      has_keyboard_key = false;
    }
  }
}

bool is_button_pushed() {
 return !gpio_get(BUTTON_A_GPIO);
}

void hid_task(void)
{
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms ) return;
  start_ms += interval_ms;

  uint32_t const btn = is_button_pushed();

  if ( tud_suspended() && btn ) {
    tud_remote_wakeup();
  } else {
    send_hid_report(REPORT_ID_KEYBOARD, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) report;
  (void) len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // @TODO - Implement
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}
