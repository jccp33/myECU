#include <cstdint>

extern "C" {
    extern std::uint32_t _estack;
    extern std::uint32_t _sidata;
    extern std::uint32_t _sdata;
    extern std::uint32_t _edata;
    extern std::uint32_t _sbss;
    extern std::uint32_t _ebss;
    int main();
    void Reset_Handler();
    void Default_Handler();
    void SysTick_Handler();
    void TIM3_IRQHandler();
}

using IsrHandler = void(*)();

struct VectorTable {
    // Cortex-M3 exceptions
    void *initStackPointer;
    IsrHandler reset;
    IsrHandler nmi;
    IsrHandler hardFault;
    IsrHandler memManage;
    IsrHandler busFault;
    IsrHandler usageFault;
    IsrHandler reserved7;
    IsrHandler reserved8;
    IsrHandler reserved9;
    IsrHandler reserved10;
    IsrHandler svCall;
    IsrHandler debugMonitor;
    IsrHandler reserved13;
    IsrHandler pendSV;
    IsrHandler sysTick;
    // STM32F103 external interrupts
    IsrHandler wwdg;              // IRQ0
    IsrHandler pvd;               // IRQ1
    IsrHandler tamper;            // IRQ2
    IsrHandler rtc;               // IRQ3
    IsrHandler flash;             // IRQ4
    IsrHandler rcc;               // IRQ5
    IsrHandler exti0;             // IRQ6
    IsrHandler exti1;             // IRQ7
    IsrHandler exti2;             // IRQ8
    IsrHandler exti3;             // IRQ9
    IsrHandler exti4;             // IRQ10
    IsrHandler dma1Channel1;      // IRQ11
    IsrHandler dma1Channel2;      // IRQ12
    IsrHandler dma1Channel3;      // IRQ13
    IsrHandler dma1Channel4;      // IRQ14
    IsrHandler dma1Channel5;      // IRQ15
    IsrHandler dma1Channel6;      // IRQ16
    IsrHandler dma1Channel7;      // IRQ17
    IsrHandler adc1_2;            // IRQ18
    IsrHandler usbHpCanTx;        // IRQ19
    IsrHandler usbLpCanRx0;       // IRQ20
    IsrHandler canRx1;            // IRQ21
    IsrHandler canSce;            // IRQ22
    IsrHandler exti9_5;           // IRQ23
    IsrHandler tim1Brk;           // IRQ24
    IsrHandler tim1Up;            // IRQ25
    IsrHandler tim1TrgCom;        // IRQ26
    IsrHandler tim1Cc;            // IRQ27
    IsrHandler tim2;              // IRQ28
    IsrHandler tim3;              // IRQ29
};

__attribute__((used, section(".isr_vector"))) const VectorTable vectorTable = {
    &_estack,               // 0 = Initial Stack Pointer   
    Reset_Handler,          // 1 = Reset
    Default_Handler,        // 2 = NMI
    Default_Handler,        // 3 = HardFault
    Default_Handler,        // 4 = MemManag
    Default_Handler,        // 5 = BusFault
    Default_Handler,        // 6 = UsageFault
    nullptr,                // 7 = Reserved
    nullptr,                // 8 = Reserved
    nullptr,                // 9 = Reserved
    nullptr,                // 10 = Reserved
    Default_Handler,        // 11 = SVCall
    Default_Handler,        // 12 = DebugMonitor
    nullptr,                // 13 = Reserved
    Default_Handler,        // 14 = PendSV
    SysTick_Handler,        // 15 = SysTick
    // External interrupts
    Default_Handler,        // IRQ0  = WWDG
    Default_Handler,        // IRQ1  = PVD
    Default_Handler,        // IRQ2  = TAMPER
    Default_Handler,        // IRQ3  = RTC
    Default_Handler,        // IRQ4  = FLASH
    Default_Handler,        // IRQ5  = RCC
    Default_Handler,        // IRQ6  = EXTI0
    Default_Handler,        // IRQ7  = EXTI1
    Default_Handler,        // IRQ8  = EXTI2
    Default_Handler,        // IRQ9  = EXTI3
    Default_Handler,        // IRQ10 = EXTI4
    Default_Handler,        // IRQ11 = DMA1_Channel1
    Default_Handler,        // IRQ12 = DMA1_Channel2
    Default_Handler,        // IRQ13 = DMA1_Channel3
    Default_Handler,        // IRQ14 = DMA1_Channel4
    Default_Handler,        // IRQ15 = DMA1_Channel5
    Default_Handler,        // IRQ16 = DMA1_Channel6
    Default_Handler,        // IRQ17 = DMA1_Channel7
    Default_Handler,        // IRQ18 = ADC1_2
    Default_Handler,        // IRQ19 = USB_HP_CAN_TX
    Default_Handler,        // IRQ20 = USB_LP_CAN_RX0
    Default_Handler,        // IRQ21 = CAN_RX1
    Default_Handler,        // IRQ22 = CAN_SCE
    Default_Handler,        // IRQ23 = EXTI9_5
    Default_Handler,        // IRQ24 = TIM1_BRK
    Default_Handler,        // IRQ25 = TIM1_UP
    Default_Handler,        // IRQ26 = TIM1_TRG_COM
    Default_Handler,        // IRQ27 = TIM1_CC
    Default_Handler,        // IRQ28 = TIM2
    TIM3_IRQHandler         // IRQ29 = TIM3
};

extern "C" void Reset_Handler(){
    std::uint32_t *src = &_sidata;
    std::uint32_t *dst = &_sdata;
    while(dst < &_edata){
        *dst = *src;
        ++dst;
        ++src;
    }
    dst = &_sbss;
    while(dst < &_ebss){
        *dst = 0;
        ++dst;
    }
    main();
    while(true){}
}

extern "C" void Default_Handler(){
    while (true){}
}
