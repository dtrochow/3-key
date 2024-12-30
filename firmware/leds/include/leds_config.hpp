#pragma once

#include "pico/stdlib.h"

#define DEFAULT_LED_PIN 18
#define DEFAULT_FREQ 800000

struct Led {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    Led(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0) : red(r), green(g), blue(b) {}
};

enum Color { Red, Green, Blue, None };
