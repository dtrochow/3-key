#include <algorithm>
#include <limits>
#include <span>
#include <sstream>
#include <vector>

#include "keys_config.hpp"
#include "pico/bootrom.h"
#include "terminal.hpp"

#define PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK 0u

Terminal::Terminal(Storage& storage, KeysConfig& keys) : storage(storage), keys(keys) {
    buffer += start_string;
}

std::span<uint8_t> Terminal::terminal(char new_char) {
    bool is_enter                    = is_enter_pressed(new_char);
    static std::string output_buffer = "";

    if (is_backspace_pressed(new_char)) {
        if (buffer.size() > (start_string.size() / sizeof(char))) {
            buffer.pop_back();
        }
    } else if (is_enter) {
        const std::string command = buffer.substr(start_string.length());
        buffer.clear();
        (void)handle_command(command);
        buffer += "\n" + start_string;
    } else if (is_new_valid_char(new_char)) {
        buffer += new_char;
    }

    output_buffer = buffer;

    if (is_enter)
        buffer = start_string;

    return std::span<uint8_t>(reinterpret_cast<uint8_t*>(output_buffer.data()), output_buffer.size());
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

bool Terminal::handle_command(const std::string& command_str) {
    std::istringstream iss(command_str);
    std::string command_name;
    iss >> command_name;

    std::vector<std::string> parameters;
    std::string param;
    while (iss >> param) {
        parameters.push_back(param);
    }

    const auto it     = command_map.find(command_name);
    const Command cmd = (it != command_map.end()) ? it->second : Command::UNKNOWN;
    return dispatch_command(cmd, parameters);
}

bool Terminal::dispatch_command(Command command, const std::vector<std::string>& params) {
    switch (command) {
        case Command::RESET: {
            reset_to_bootloader();
            return true;
        }
        case Command::ERASE: {
            storage.erase();
            add_log("Flash storage erased");
            return true;
        }
        case Command::CHANGE_COLOR: {
            return handle_change_color_command(params);
        }
        default: return false;
    }
}

bool Terminal::is_valid_number(const std::string& str) const {
    return !str.empty() && std::all_of(str.begin(), str.end(), ::isdigit);
}

bool Terminal::handle_change_color_command(const std::vector<std::string>& params) {
    if (params.size() < 2) {
        add_log("Error: change_color requires 2 parameters");
        return false;
    }

    const std::string& button_id_str = params[0];
    const uint button_id             = std::stoul(button_id_str);
    if (!is_valid_number(button_id_str) || (button_id >= keys.get_keys_count())) {
        add_log("Error: Invalid button ID");
        return false;
    }

    const std::string& color_name = params[1];
    Color color;
    if (color_name == "red") {
        color = Color::Red;
    } else if (color_name == "green") {
        color = Color::Green;
    } else if (color_name == "blue") {
        color = Color::Blue;
    } else {
        add_log("Error: Invalid color");
        return false;
    }

    keys.set_key_color(button_id, color);
    add_log("Changing button " + std::to_string(button_id) + " color to " + color_name);

    return true;
}

void Terminal::reset_to_bootloader() const {
    reset_usb_boot(1u, PICO_STDIO_USB_RESET_BOOTSEL_INTERFACE_DISABLE_MASK);
}

void Terminal::add_log(std::string log) {
    buffer += "\r\n" + log;
}
