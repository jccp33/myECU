#include "led.hpp"
#include <cstdint>

namespace {
    constexpr std::uintptr_t RCC_BASE    = 0x40021000UL;
    constexpr std::uintptr_t GPIOC_BASE  = 0x40011000UL;
    constexpr std::uintptr_t RCC_APB2ENR = RCC_BASE + 0x18UL;
    constexpr std::uintptr_t GPIOC_CRH   = GPIOC_BASE + 0x04UL;
    constexpr std::uintptr_t GPIOC_BSRR  = GPIOC_BASE + 0x10UL;
    constexpr std::uintptr_t GPIOC_BRR   = GPIOC_BASE + 0x14UL;
    volatile std::uint32_t& reg(std::uintptr_t address)
    {
        return *reinterpret_cast<volatile std::uint32_t*>(address);
    }
}

namespace platform {
    void initLed()
    {
        // Enabled GPIOC clock
        reg(RCC_APB2ENR) |= (1UL << 4);
        // PC13: output push-pull, 2 MHz.
        reg(GPIOC_CRH) &= ~(0xFUL << 20);
        reg(GPIOC_CRH) |=  (0x2UL << 20);
        // turn off led
        turnLedOff();
    }

    void turnLedOn()
    {
        // PC13 avitve on LOW
        reg(GPIOC_BRR) = (1UL << 13);
    }

    void turnLedOff()
    {
        reg(GPIOC_BSRR) = (1UL << 13);
    }

    void toggleLed()
    {
        constexpr std::uintptr_t GPIOC_ODR = GPIOC_BASE + 0x0CUL;
        if (reg(GPIOC_ODR) & (1UL << 13))
        {
            turnLedOn();
        }
        else
        {
            turnLedOff();
        }
    }
}
