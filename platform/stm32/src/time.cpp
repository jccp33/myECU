#include "time.hpp"
#include <cstdint>

namespace {
    constexpr std::uintptr_t SYST_CSR = 0xE000E010UL;
    constexpr std::uintptr_t SYST_RVR = 0xE000E014UL;
    constexpr std::uintptr_t SYST_CVR = 0xE000E018UL;
    volatile std::uint32_t ticks = 0;
    volatile std::uint32_t &reg(std::uintptr_t address){
        return *reinterpret_cast<volatile std::uint32_t *>(address);
    }
}

namespace platform{
    void initTime(){
        // CPU using HSI = 8 MHz
        // 1 interrupt every 1 ms
        // 8,000,000 / 1000 = 8,000 cycles
        // SysTick count from RELOAD to 0
        reg(SYST_RVR) = 8000 - 1;
        // clean current value
        reg(SYST_CVR) = 0;
        // CSR:
        // bit 0: ENABLE    = 1
        // bit 1: TICKINT   = 1
        // bit 2: CLKSOURCE = 1 -> CPU clock
        reg(SYST_CSR) = (1UL << 0) | (1UL << 1) | (1UL << 2);
    }

    std::uint32_t millis(){
        return ticks;
    }
}

extern "C" void SysTick_Handler(){
    ++ticks;
}
