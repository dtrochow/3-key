#include "terminal.hpp"
#include "pico/bootrom.h"
#include <cstddef>

#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u

Terminal::Terminal() {
    buffer += start_string;
    buff_size_bytes = start_string.length();
}

size_t Terminal::get_buff_size() const {
    return buff_size_bytes;
}

uint8_t* Terminal::terminal(char new_char) {
    bool is_enter                    = is_enter_pressed(new_char);
    static std::string output_buffer = "";

    if (is_backspace_pressed(new_char)) {
        if (buffer.size() > (start_string.size() / sizeof(char))) {
            buffer.pop_back();
        }
    } else if (is_enter) {
        const std::string command = buffer.substr(start_string.length());
        buffer.clear();
        if (!handle_command(command)) {
            buffer += "\n\rUnsupported command";
        }
        buffer += "\n" + start_string;
    } else if (is_new_valid_char(new_char)) {
        buffer += new_char;
    }

    output_buffer   = buffer;
    buff_size_bytes = output_buffer.length();

    if (is_enter)
        buffer = start_string;

    return reinterpret_cast<uint8_t*>(output_buffer.data());
}

bool Terminal::is_enter_pressed(char& ch) const {
    return (ch == '\r' || ch == '\n');
}

bool Terminal::is_backspace_pressed(char& ch) const {
    return (ch == '\b' || ch == 127);
}

bool Terminal::is_new_valid_char(char& ch) const {
    return (isprint(ch) && buffer.size() < max_chars + 6);
}

bool Terminal::handle_command(const std::string& command_str) const {
    auto it           = command_map.find(command_str);
    const Command cmd = (it != command_map.end()) ? it->second : Command::UNKNOWN;
    return dispatch_command(cmd);
}

bool Terminal::dispatch_command(Command command) const {
    switch (command) {
        case Command::RESET: {
            reset_to_bootloader();
            return true;
        }
        case Command::SAVE: {
            /* @TODO */
            return true;
        }
        case Command::READ: {
            /* @TODO */
            return true;
        }
        case Command::ERASE: {
            /* @TODO */
            return true;
        }
        default: return false;
    }
}

void Terminal::reset_to_bootloader() const {
    reset_usb_boot(1u, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
}
