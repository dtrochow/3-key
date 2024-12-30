#pragma once

#include <vector>

#include "buttons.hpp"
#include "hardware/pio.h"
#include "leds_config.hpp"

class Leds {
  public:
    Leds(uint leds_count, PIO pio = pio0, uint pin = DEFAULT_LED_PIN, float freq = DEFAULT_FREQ);
    ~Leds() = default;

  private:
    uint leds_count;
    float w_freq;
    uint w_pin;
    uint offset;
    PIO pio;
    uint sm;
    std::vector<Led> leds;

  public:
    void init();
    void set_led_color(uint led_id, Color color, bool r = false);
    void set_all_color(Color color, bool r = false);
    void blink(uint led_id, Color color, uint count, float freq = 1);
    void blink(Color color, uint count, float freq = 1);
    void refresh();
    void disable_all(bool r = false);

  private:
    void push_led(const Led& led);
};

void leds_task(Leds& leds, const Buttons& buttons);
