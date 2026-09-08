#include "led.hpp"
#include "time.hpp"
#include <cstdint>

int main(){
    platform::initLed();
    platform::initTime();
    std::uint32_t prevTime = 0;
    while(true){
        const std::uint32_t now = platform::millis();
        if((now - prevTime) >= 500U){
            prevTime = now;
            platform::toggleLed();
        }
    }
}
