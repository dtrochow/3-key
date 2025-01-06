#include "cdc.hpp"
#include "terminal.hpp"

void CdcDevice::task() const {
    while (tud_cdc_available()) {
        const char c = tud_cdc_read_char();

        const std::span<uint8_t> buffer_span = t.terminal(c);

        tud_cdc_write(buffer_span.data(), buffer_span.size());
        tud_cdc_write_flush();
    }
}

void CdcDevice::log(const char* message) const {
    tud_cdc_write_str(message);
    tud_cdc_write_str("\n\r");
    tud_cdc_write_flush();
}
