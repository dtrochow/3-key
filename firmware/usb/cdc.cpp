#include <cstddef>
#include <cstring>

#include "cdc.hpp"
#include "class/cdc/cdc_device.h"
#include "pico/bootrom.h"

#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u

CdcDevice::CdcDevice(size_t input_buffer_size) : input_buffer_size(input_buffer_size) {
    input_buffer.resize(input_buffer_size, 0);
}

void CdcDevice::task() {
    terminal();
}

void CdcDevice::log(const char* message) {
    tud_cdc_write_str(message);
    tud_cdc_write_str("\r\n");
    tud_cdc_write_flush();
}

bool CdcDevice::handle_command() {
    input_buffer[buffer_index] = '\0';
    std::string command(input_buffer.begin(), input_buffer.begin() + buffer_index);

    auto it = command_map.find(command);
    if (it == command_map.end()) {
        buffer_index = 0;
        return false;
    }

    bool success = dispatch_command(it->second);

    buffer_index = 0;
    return success;
}

void CdcDevice::terminal() {
    while (tud_cdc_available()) {
        char c = tud_cdc_read_char();

        tud_cdc_write_char(c);

        // ENTER pressed
        if (c == '\n' || c == '\r') {
            if (!handle_command()) {
                log("Unsupported command");
            }
        } else {
            if (buffer_index < input_buffer_size - 1) {
                input_buffer[buffer_index++] = c;
            } else {
                log("Error: Input too long!");
                buffer_index = 0;
            }
        }
        tud_cdc_write_flush();
    }
}

bool CdcDevice::dispatch_command(Command command) {
    switch (command) {
        case Command::RESET: {
            reset_to_bootloader();
            return true;
        }
        default: return false;
    }
}

void CdcDevice::reset_to_bootloader() {
    reset_usb_boot(1u, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
}
