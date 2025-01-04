#pragma once

#include "class/cdc/cdc_device.h"
#include "pico/stdlib.h"

#include "terminal.hpp"

void cdc_task();

class CdcDevice {
  public:
    explicit CdcDevice(Terminal& t) : t(t) {};
    ~CdcDevice() = default;

    void task() const;
    void log(const char* message) const;

  private:
    Terminal& t;
};
