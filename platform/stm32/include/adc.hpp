#pragma once 

#include <stdint.h>

namespace platform {
    void initAdc();
    uint16_t readAdc(uint8_t channel);
}
