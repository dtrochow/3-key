#include <map>
#include <string>
#include <vector>

#include "class/cdc/cdc_device.h"
#include "pico/stdlib.h"

void cdc_task();

class CdcDevice {
  public:
    CdcDevice(size_t input_buffer_size = 128);
    ~CdcDevice() = default;

    void task();
    void log(const char* message);

  private:
    std::vector<char> input_buffer;
    uint32_t buffer_index;
    size_t input_buffer_size;
    void reset_to_bootloader();
    void terminal();
    bool handle_command();

    enum class Command { RESET, UNKNOWN };

    std::map<std::string, Command> command_map = {
        { "reset", Command::RESET },
    };

    bool dispatch_command(Command command);
};
