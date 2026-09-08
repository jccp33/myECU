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
}

using IsrHandler = void(*)();

struct VectorTable {
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
};

__attribute__((used, section(".isr_vector"))) 
const VectorTable vectorTable = {
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
    SysTick_Handler         // 15 = SysTick
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
