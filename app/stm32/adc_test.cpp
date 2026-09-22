#include "adc.hpp"
#include "led.hpp"

volatile uint16_t g_adcValue = 0U;

int main(){
    platform::initAdc();
    
    while(true){
        g_adcValue = platform::readAdc(0U);
    }

    return 0;
}
