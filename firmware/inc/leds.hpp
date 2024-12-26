#pragma once

#include <vector>

#include "hardware/pio.h"

#define DEFAULT_LED_PIN 18
#define DEFAULT_FREQ 800000

struct Led {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Led(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : red(r), green(g), blue(b) {}
};

enum Color {
    Red,
    Green,
    Blue,
    None
};

class Leds {
public:
    Leds(uint leds_count, PIO pio = pio0, uint pin = DEFAULT_LED_PIN, float freq = DEFAULT_FREQ);
    ~Leds() {};
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
