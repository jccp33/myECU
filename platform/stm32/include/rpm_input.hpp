#pragma once

#include <cstdint>

namespace platform {
    void initRpmInput();
    bool readRpmPeriodUs(std::uint32_t &periodUs);
}
