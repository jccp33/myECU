#include "adc.hpp"
#include "led.hpp"

volatile uint16_t g_adcValue = 0U;

int main(){
    //platform::initLed();
    platform::initAdc();
    
    while(true){
        //const uint16_t adcValue = platform::readAdc();
        //if(adcValue >= 2048U){
        //    platform::turnLedOn();
        //}else{
        //    platform::turnLedOff();
        //}
        g_adcValue = platform::readAdc();
    }

    return 0;
}
