#include <pico/time.h>
#include <ranges>

#include "leds.hpp"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

Leds::Leds(uint leds_count, PIO pio, uint pin, float freq)
: leds_count(leds_count), leds(leds_count, Led(0, 0, 0)), pio(pio), w_pin(pin), w_freq(freq), sm(0) {
}

void Leds::init() {
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, w_pin, w_freq, false);
    refresh();
    sleep_ms(10);
}

void Leds::set_led_color(uint led_id, Color color, bool r) {
    if (led_id >= leds.size()) {
        return;
    }
    switch (color) {
    case Red: leds[led_id] = Led(255, 0, 0); break;
    case Green: leds[led_id] = Led(0, 255, 0); break;
    case Blue: leds[led_id] = Led(0, 0, 255); break;
    case None: leds[led_id] = Led(0, 0, 0); break;
    }
    if (r)
        refresh();
}

void Leds::set_all_color(Color color, bool r) {
    for (int id = 0; id < leds.size(); id++) {
        set_led_color(id, color);
    }
    if (r)
        refresh();
}

void Leds::disable_all(bool r) {
    for (int id = 0; id < leds.size(); id++) {
        set_led_color(id, Color::None);
    }
    if (r)
        refresh();
}

void Leds::push_led(const Led& led) {
    uint32_t color = (static_cast<uint32_t>(led.red) << 16) |
    (static_cast<uint32_t>(led.green) << 8) | (static_cast<uint32_t>(led.blue));

    pio_sm_put_blocking(pio, sm, color << 8u);
}

void Leds::refresh() {
    for (const auto led : std::ranges::reverse_view(leds)) {
        push_led(led);
    }
}

void Leds::blink(Color color, uint count, float freq) {
    uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (int i = 0; i < count; i++) {
        set_all_color(color, true);
        sleep_ms(delay);
        set_all_color(Color::None, true);
        sleep_ms(delay);
    }
}

void Leds::blink(uint led_id, Color color, uint count, float freq) {
    uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (int i = 0; i < count; i++) {
        set_led_color(led_id, color, true);
        sleep_ms(delay);
        set_led_color(led_id, Color::None, true);
        sleep_ms(delay);
    }
}
