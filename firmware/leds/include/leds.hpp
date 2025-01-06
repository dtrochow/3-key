#pragma once

#include <vector>

#include "buttons.hpp"
#include "hardware/pio.h"
#include "keys_config.hpp"
#include "leds_config.hpp"

class Leds {
  public:
    Leds(uint leds_count, KeysConfig& keys, PIO pio = pio0, uint pin = DEFAULT_LED_PIN, float freq = DEFAULT_FREQ);
    ~Leds() = default;

  private:
    KeysConfig& keys;

    uint leds_count;
    float w_freq;
    uint w_pin;
    uint offset;
    PIO pio;
    uint sm;
    std::vector<Led> leds;

  public:
    void init();
    void enable(uint led_id, bool r = false);
    void disable(uint led_id, bool r = false);
    void blink(uint led_id, uint count, float freq = 1);
    void blink(uint count, float freq = 1);
    void refresh() const;
    void enable_all(bool r = false);
    void disable_all(bool r = false);

  private:
    void push_led(const Led& led) const;
};

void leds_task(Leds& leds, const Buttons& buttons);
