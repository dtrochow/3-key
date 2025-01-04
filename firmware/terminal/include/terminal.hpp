#pragma once

#include <map>
#include <string>

#include "pico/stdlib.h"
#include "storage.hpp"

enum class Command {
    RESET,
    SAVE,
    READ,
    ERASE,
    UNKNOWN,
};

class Terminal {
  private:
    static constexpr std::string start_string = "\r3-key>";
    static constexpr size_t max_chars         = 128;
    std::string buffer;
    size_t buff_size_bytes;
    Storage& storage;

  public:
    Terminal(Storage& storage);
    ~Terminal() = default;
    uint8_t* terminal(char new_char);
    size_t get_buff_size() const;

  private:
    std::map<std::string, Command> command_map = {
        { "reset", Command::RESET },
        { "save", Command::SAVE },
        { "read", Command::READ },
        { "erase", Command::ERASE },
    };

    bool dispatch_command(Command command) const;
    bool handle_command(const std::string& command_str) const;
    bool is_enter_pressed(char& ch) const;
    bool is_backspace_pressed(char& ch) const;
    bool is_new_valid_char(char& ch) const;

    void reset_to_bootloader() const;
};
