#include "led.hpp"
#include <cstdint>

namespace {
    // STM32F103 peripheral base addresses.
    constexpr std::uintptr_t RCC_BASE   = 0x40021000UL;
    constexpr std::uintptr_t GPIOB_BASE = 0x40010C00UL;
    // RCC registers.
    constexpr std::uintptr_t RCC_APB2ENR = RCC_BASE + 0x18UL;
    // GPIOB registers.
    constexpr std::uintptr_t GPIOB_CRL  = GPIOB_BASE + 0x00UL;
    constexpr std::uintptr_t GPIOB_CRH  = GPIOB_BASE + 0x04UL;
    constexpr std::uintptr_t GPIOB_BSRR = GPIOB_BASE + 0x10UL;
    constexpr std::uintptr_t GPIOB_BRR  = GPIOB_BASE + 0x14UL;
    // Physical Blue Pill
    constexpr std::uint32_t LED_GREEN_PIN_B5 = 5U;
    constexpr std::uint32_t LED_GREEN_PIN_B5_SHIFT = LED_GREEN_PIN_B5 * 4U;
    constexpr std::uint32_t LED_YELLOW_PIN_B6 = 6U;
    constexpr std::uint32_t LED_YELLOW_PIN_B6_SHIFT = LED_YELLOW_PIN_B6 * 4U;
    constexpr std::uint32_t LED_RED_PIN_B7 = 7U;
    constexpr std::uint32_t LED_RED_PIN_B7_SHIFT = LED_RED_PIN_B7 * 4U;
    constexpr std::uint32_t LED_BLUE_PIN_B8 = 8U;
    constexpr std::uint32_t LED_BLUE_PIN_B8_SHIFT = (LED_BLUE_PIN_B8 - 8U) * 4U;
    // register
    volatile std::uint32_t& reg(std::uintptr_t address)
    {
        return *reinterpret_cast<volatile std::uint32_t*>(address);
    }
}

namespace platform {
    void initLeds()
    {
        reg(RCC_APB2ENR) |= (1UL << 3);
        reg(GPIOB_CRL) &= ~(0xFUL << LED_GREEN_PIN_B5_SHIFT);
        reg(GPIOB_CRL) |=  (0x2UL << LED_GREEN_PIN_B5_SHIFT);
        reg(GPIOB_CRL) &= ~(0xFUL << LED_YELLOW_PIN_B6_SHIFT);
        reg(GPIOB_CRL) |=  (0x2UL << LED_YELLOW_PIN_B6_SHIFT);
        reg(GPIOB_CRL) &= ~(0xFUL << LED_RED_PIN_B7_SHIFT);
        reg(GPIOB_CRL) |=  (0x2UL << LED_RED_PIN_B7_SHIFT);
        reg(GPIOB_CRH) &= ~(0xFUL << LED_BLUE_PIN_B8_SHIFT);
        reg(GPIOB_CRH) |=  (0x2UL << LED_BLUE_PIN_B8_SHIFT);
        turnAllLedsOff();
    }

    void turnLedOn(Led led)
    {
        switch(led){
            case Led::GREEN:
                reg(GPIOB_BSRR) = (1UL << LED_GREEN_PIN_B5);
                break;
            case Led::YELLOW:
                reg(GPIOB_BSRR) = (1UL << LED_YELLOW_PIN_B6);
                break;
            case Led::RED:
                reg(GPIOB_BSRR) = (1UL << LED_RED_PIN_B7);
                break;
            case Led::BLUE:
                reg(GPIOB_BSRR) = (1UL << LED_BLUE_PIN_B8);
                break;
            default:
                break;
        }
    }

    void turnLedOff(Led led)
    {
        switch(led){
            case Led::GREEN:
                reg(GPIOB_BRR) = (1UL << LED_GREEN_PIN_B5);
                break;
            case Led::YELLOW:
                reg(GPIOB_BRR) = (1UL << LED_YELLOW_PIN_B6);
                break;
            case Led::RED:
                reg(GPIOB_BRR) = (1UL << LED_RED_PIN_B7);
                break;
            case Led::BLUE:
                reg(GPIOB_BRR) = (1UL << LED_BLUE_PIN_B8);
                break;
            default:
                break;
        }
    }

    void turnAllLedsOff()
    {
        reg(GPIOB_BRR) = (1UL << LED_GREEN_PIN_B5);
        reg(GPIOB_BRR) = (1UL << LED_YELLOW_PIN_B6);
        reg(GPIOB_BRR) = (1UL << LED_RED_PIN_B7);
        reg(GPIOB_BRR) = (1UL << LED_BLUE_PIN_B8);
    }
}
