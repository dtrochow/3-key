#include <ranges>

#include "leds.hpp"
#include "leds_config.hpp"
#include "pico/stdlib.h"
#include "ws2812.pio.h"

Leds::Leds(uint leds_count, KeysConfig& keys, PIO pio, uint pin, float freq)
: leds_count(leds_count), keys(keys), leds(leds_count, Led(0, 0, 0)), pio(pio), w_pin(pin),
  w_freq(freq), sm(0) {}

void Leds::init() {
    offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, w_pin, w_freq, false);
    refresh();
    sleep_ms(10);
}

void Leds::enable(uint led_id, bool r) {
    if (led_id >= leds.size()) {
        return;
    }
    switch (keys.get_key_color(led_id)) {
        case Red: leds[led_id] = Led(255, 0, 0); break;
        case Green: leds[led_id] = Led(0, 255, 0); break;
        case Blue: leds[led_id] = Led(0, 0, 255); break;
        case None: leds[led_id] = Led(0, 0, 0); break;
    }
    if (r)
        refresh();
}

void Leds::disable(uint led_id, bool r) {
    if (led_id >= leds.size()) {
        return;
    }

    leds[led_id] = Led(0, 0, 0);
    if (r)
        refresh();
}

void Leds::enable_all(bool r) {
    for (int id = 0; id < leds.size(); id++) {
        enable(id);
    }
    if (r)
        refresh();
}

void Leds::disable_all(bool r) {
    for (int id = 0; id < leds.size(); id++) {
        disable(id);
    }
    if (r)
        refresh();
}

void Leds::push_led(const Led& led) const {
    const uint32_t color = (static_cast<uint32_t>(led.red) << 16) |
        (static_cast<uint32_t>(led.green) << 8) | (static_cast<uint32_t>(led.blue));

    pio_sm_put_blocking(pio, sm, color << 8u);
}

void Leds::refresh() const {
    for (const auto led : std::ranges::reverse_view(leds)) {
        push_led(led);
    }
}

void Leds::blink(uint count, float freq) {
    const uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (int i = 0; i < count; i++) {
        enable_all(true);
        sleep_ms(delay);
        disable_all(true);
        sleep_ms(delay);
    }
}

void Leds::blink(uint led_id, uint count, float freq) {
    const uint32_t delay = static_cast<uint32_t>(((1 / freq) / 2) * 1000);
    for (int i = 0; i < count; i++) {
        enable(led_id, true);
        sleep_ms(delay);
        disable(led_id, true);
        sleep_ms(delay);
    }
}

void leds_task(Leds& leds, const Buttons& buttons) {
    const std::vector<Button> btns = buttons.get_btns();
    for (const auto& btn : btns) {
        const uint btn_id = buttons.get_btn_id(btn);
        if (buttons.is_btn_pressed(btn)) {
            leds.enable(btn_id);
        } else {
            leds.disable(btn_id);
        }
    }
    leds.refresh();
}
