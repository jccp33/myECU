#include <cstdint>

namespace
{
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
    // reg
    volatile std::uint32_t& reg(std::uintptr_t address)
    {
        return *reinterpret_cast<volatile std::uint32_t*>(address);
    }
}

int main()
{
    const short led_option = 3;
    // Enable GPIOB peripheral clock.
    reg(RCC_APB2ENR) |= (1UL << 3);

    switch (led_option){
        case 0:
            reg(GPIOB_CRL) &= ~(0xFUL << LED_GREEN_PIN_B5_SHIFT);
            reg(GPIOB_CRL) |=  (0x2UL << LED_GREEN_PIN_B5_SHIFT);
            while (true){
                reg(GPIOB_BSRR) = (1UL << LED_GREEN_PIN_B5);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
                reg(GPIOB_BRR) = (1UL << LED_GREEN_PIN_B5);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
            }
            break;
        case 1:
            reg(GPIOB_CRL) &= ~(0xFUL << LED_YELLOW_PIN_B6_SHIFT);
            reg(GPIOB_CRL) |=  (0x2UL << LED_YELLOW_PIN_B6_SHIFT);
            while (true){
                reg(GPIOB_BSRR) = (1UL << LED_YELLOW_PIN_B6);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
                reg(GPIOB_BRR) = (1UL << LED_YELLOW_PIN_B6);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
            }
            break;
        case 2:
            reg(GPIOB_CRL) &= ~(0xFUL << LED_RED_PIN_B7_SHIFT);
            reg(GPIOB_CRL) |=  (0x2UL << LED_RED_PIN_B7_SHIFT);
            while (true){
                reg(GPIOB_BSRR) = (1UL << LED_RED_PIN_B7);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
                reg(GPIOB_BRR) = (1UL << LED_RED_PIN_B7);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
            }
            break;
        case 3:
            reg(GPIOB_CRH) &= ~(0xFUL << LED_BLUE_PIN_B8_SHIFT);
            reg(GPIOB_CRH) |=  (0x2UL << LED_BLUE_PIN_B8_SHIFT);
            while (true){
                reg(GPIOB_BSRR) = (1UL << LED_BLUE_PIN_B8);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
                reg(GPIOB_BRR) = (1UL << LED_BLUE_PIN_B8);
                for (volatile std::uint32_t i = 0U; i < 500000U; ++i){}
            }
            break;
        default:
            break;
    }

    // finish
    return 0;
}
