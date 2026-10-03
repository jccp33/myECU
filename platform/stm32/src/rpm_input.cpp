#include "rpm_input.hpp"

namespace platform {
    namespace {
        // RCC
        volatile std::uint32_t* const RCC_APB1ENR = reinterpret_cast<volatile std::uint32_t*>(0x4002101CUL);
        volatile std::uint32_t* const RCC_APB2ENR = reinterpret_cast<volatile std::uint32_t*>(0x40021018UL);
        // GPIOA
        volatile std::uint32_t* const GPIOA_CRL = reinterpret_cast<volatile std::uint32_t*>(0x40010800UL);
        // TIM3
        volatile std::uint32_t* const TIM3_CR1 = reinterpret_cast<volatile std::uint32_t*>(0x40000400UL);
        volatile std::uint32_t* const TIM3_DIER = reinterpret_cast<volatile std::uint32_t*>(0x4000040CUL);
        volatile std::uint32_t* const TIM3_SR = reinterpret_cast<volatile std::uint32_t*>(0x40000410UL);
        volatile std::uint32_t* const TIM3_CCMR1 = reinterpret_cast<volatile std::uint32_t*>(0x40000418UL);
        volatile std::uint32_t* const TIM3_CCER = reinterpret_cast<volatile std::uint32_t*>(0x40000420UL);
        volatile std::uint32_t* const TIM3_CNT = reinterpret_cast<volatile std::uint32_t*>(0x40000424UL);
        volatile std::uint32_t* const TIM3_PSC = reinterpret_cast<volatile std::uint32_t*>(0x40000428UL);
        volatile std::uint32_t* const TIM3_ARR = reinterpret_cast<volatile std::uint32_t*>(0x4000042CUL);
        volatile std::uint32_t* const TIM3_CCR1 = reinterpret_cast<volatile std::uint32_t*>(0x40000434UL);
        volatile std::uint32_t* const NVIC_ISER0 = reinterpret_cast<volatile std::uint32_t*>(0xE000E100UL);
        // Software state
        std::uint32_t overflowCount = 0U;
        std::uint32_t previousCapture = 0U;
        volatile std::uint32_t periodUs = 0U;
        volatile bool periodAvailable = false;
        bool firstCaptureReceived = false;
        // functions
        std::uint32_t disableInterrupts(){
            std::uint32_t primask;
            asm volatile(
                "MRS %0, PRIMASK\n"
                "CPSID i"
                : "=r"(primask)
                :
                : "memory"
            );
            return primask;
        }
        void restoreInterrupts(std::uint32_t primask){
            asm volatile(
                "MSR PRIMASK, %0"
                :
                : "r"(primask)
                : "memory"
            );
        }
    }

    void initRpmInput() {
        // GPIOA clock
        *RCC_APB2ENR |= (1U << 2);
        // PA6 floating input
        *GPIOA_CRL &= ~(0xFU << 24);
        *GPIOA_CRL |=  (0x4U << 24);
        // TIM3 clock
        *RCC_APB1ENR |= (1U << 1);
        // Stop TIM3
        *TIM3_CR1 &= ~(1U << 0);
        *TIM3_PSC = 7U;
        *TIM3_ARR = 0xFFFFU;
        // Reset counter
        *TIM3_CNT = 0U;
        *TIM3_CCMR1 &= ~(0x3U << 0);
        *TIM3_CCMR1 |=  (0x1U << 0);
        // Rising edge
        *TIM3_CCER &= ~(1U << 1);
        // Enable CH1 capture
        *TIM3_CCER |= (1U << 0);
        // Clear status flags
        *TIM3_SR = 0U;
        // Enable TIM3 interrupts:
        *TIM3_DIER |= (1U << 0);
        *TIM3_DIER |= (1U << 1);
        // Enable TIM3 interrupt in NVIC.
        *NVIC_ISER0 = (1U << 29);
        // Reset software state
        overflowCount = 0U;
        previousCapture = 0U;
        periodUs = 0U;
        firstCaptureReceived = false;
        periodAvailable = false;
        // Start TIM3
        *TIM3_CR1 |= (1U << 0);
    }

    void updateRpmInput() {
        // Check timer overflow
        if ((*TIM3_SR & (1U << 0)) != 0U) {
            ++overflowCount;
            *TIM3_SR &= ~(1U << 0);
        }
        // Check input capture
        if ((*TIM3_SR & (1U << 1)) != 0U) {
            const std::uint32_t capture = *TIM3_CCR1 & 0xFFFFU;
            const std::uint32_t timestamp = (overflowCount << 16) | capture;
            // Clear capture flag
            *TIM3_SR &= ~(1U << 1);
            if (!firstCaptureReceived) {
                previousCapture = timestamp;
                firstCaptureReceived = true;
                return;
            }
            // Difference between captures
            periodUs = timestamp - previousCapture;
            previousCapture = timestamp;
            periodAvailable = true;
        }
    }

    bool readRpmPeriodUs(std::uint32_t &outputPeriodUs){
        const std::uint32_t primask = disableInterrupts();
        if (!periodAvailable){
            restoreInterrupts(primask);
            return false;
        }
        outputPeriodUs = periodUs;
        periodAvailable = false;
        restoreInterrupts(primask);
        return true;
    }
}

extern "C" void TIM3_IRQHandler()
{
    platform::updateRpmInput();
}
