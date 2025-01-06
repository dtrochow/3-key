#pragma once

#include <map>
#include <string>

#include "keys_config.hpp"
#include "pico/stdlib.h"
#include "storage.hpp"

enum class Command {
    RESET,
    ERASE,
    CHANGE_COLOR,
    UNKNOWN,
};

class Terminal {
  private:
    static constexpr std::string start_string = "\r3-key>";
    static constexpr size_t max_chars         = 128;
    std::string buffer;
    Storage& storage;

  public:
    Terminal(Storage& storage, KeysConfig& keys);
    ~Terminal() = default;
    std::span<uint8_t> terminal(char new_char);

  private:
    KeysConfig& keys;

    /* Command strings mapping */
    std::map<std::string, Command> command_map = {
        { "reset", Command::RESET },
        { "erase", Command::ERASE },
        { "color", Command::CHANGE_COLOR },
    };

    bool dispatch_command(Command command, const std::vector<std::string>& params);
    bool handle_command(const std::string& command_str);
    bool is_enter_pressed(char& ch) const;
    bool is_backspace_pressed(char& ch) const;
    bool is_new_valid_char(char& ch) const;
    bool is_valid_number(const std::string& str) const;

    void add_log(std::string log);

    void reset_to_bootloader() const;
    bool handle_change_color_command(const std::vector<std::string>& params);
};
