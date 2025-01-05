#include "pico/multicore.h"

#include "buttons.hpp"
#include "cdc.hpp"
#include "config.hpp"
#include "hid.hpp"
#include "leds.hpp"
#include "storage.hpp"
#include "terminal.hpp"
#include "tud.hpp"

Leds* g_leds       = nullptr;
Buttons* g_buttons = nullptr;

void leds_task_on_core1() {
    while (1) {
        leds_task(*g_leds, *g_buttons);
        sleep_ms(50);
    }
}

int main(void) {
    const std::vector<ButtonConfig> key_configs = {
        { 0, BUTTON_RIGHT_GPIO, Key::V, Color::Red },
        { 1, BUTTON_MIDDLE_GPIO, Key::C, Color::Green },
        { 2, BUTTON_LEFT_GPIO, Modifier::LEFT_CMD, Color::Blue },
    };

    Storage storage;

    storage.init();

    Leds leds(key_configs.size());
    leds.init();

    Buttons buttons(key_configs);
    buttons.init();

    g_buttons = &buttons;
    g_leds    = &leds;
    multicore_launch_core1(leds_task_on_core1);

    Terminal t(storage);
    CdcDevice cdc(t);

    initialize_tud();

    while (1) {
        tud_task();
        hid_task(buttons);
        cdc.task();
    }
}
