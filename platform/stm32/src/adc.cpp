#include "adc.hpp"

namespace platform
{

    namespace {
        volatile uint32_t* const RCC_APB2ENR = reinterpret_cast<volatile uint32_t*>(0x40021018UL);
        volatile uint32_t* const RCC_CFGR    = reinterpret_cast<volatile uint32_t*>(0x40021004UL);
        volatile uint32_t* const GPIOA_CRL   = reinterpret_cast<volatile uint32_t*>(0x40010800UL);
        volatile uint32_t* const ADC1_SR     = reinterpret_cast<volatile uint32_t*>(0x40012400UL);
        volatile uint32_t* const ADC1_CR2    = reinterpret_cast<volatile uint32_t*>(0x40012408UL);
        volatile uint32_t* const ADC1_SMPR2  = reinterpret_cast<volatile uint32_t*>(0x40012410UL);
        volatile uint32_t* const ADC1_SQR3   = reinterpret_cast<volatile uint32_t*>(0x40012434UL);
        volatile uint32_t* const ADC1_DR     = reinterpret_cast<volatile uint32_t*>(0x4001244CUL);
    }

    void initAdc() {
        // Enable GPIOA clock
        *RCC_APB2ENR |= (1U << 2);
        // Configure PA0, PA1 and PA2 as analog inputs
        *GPIOA_CRL &= ~(0xFU << 0);  // PA0 -> ADC1_IN0
        *GPIOA_CRL &= ~(0xFU << 4);  // PA1 -> ADC1_IN1
        *GPIOA_CRL &= ~(0xFU << 8);  // PA2 -> ADC1_IN2
        // ADC clock = PCLK2 / 6
        *RCC_CFGR &= ~(0x3U << 14);
        *RCC_CFGR |=  (0x2U << 14);
        // Enable ADC1 clock
        *RCC_APB2ENR |= (1U << 9);
        // Channel 0 sample time = 55.5 cycles
        *ADC1_SMPR2 &= ~(0x7U << 0);
        *ADC1_SMPR2 |=  (0x5U << 0);
        // Channel 1 sample time = 55.5 cycles
        *ADC1_SMPR2 &= ~(0x7U << 3);
        *ADC1_SMPR2 |=  (0x5U << 3);
        // First regular conversion = channel 0
        *ADC1_SQR3 &= ~(0x1FU << 0);
        // Select software trigger for regular conversion
        *ADC1_CR2 &= ~(0x7U << 17);
        *ADC1_CR2 |=  (0x7U << 17);
        // Enable regular conversion trigger
        *ADC1_CR2 |= (1U << 20);
        // Power on ADC1
        *ADC1_CR2 |= (1U << 0);
        // Allow ADC to stabilize after power-on
        for (volatile uint32_t i = 0; i < 1000U; ++i){}
        // Reset calibration
        *ADC1_CR2 |= (1U << 3);
        while ((*ADC1_CR2 & (1U << 3)) != 0U){}
        // Start calibration
        *ADC1_CR2 |= (1U << 2);
        while ((*ADC1_CR2 & (1U << 2)) != 0U){}
    }

    uint16_t readAdc(uint8_t channel)
    {
        // Select channel for the first regular conversion
        *ADC1_SQR3 &= ~(0x1FU << 0);
        *ADC1_SQR3 |= (static_cast<uint32_t>(channel) & 0x1FU);
        // star regular conversion by software
        *ADC1_CR2 |= (1U << 22);
        // Wait until conversion completes
        while ((*ADC1_SR & (1U << 1)) == 0U){}
        return static_cast<uint16_t>(*ADC1_DR & 0x0FFFU);
    }
}
