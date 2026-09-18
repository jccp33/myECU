
#pragma once

namespace platform{
    enum class Led{
        GREEN,
        YELLOW,
        RED,
        BLUE
    };
    void initLeds();
    void turnLedOn(Led led);
    void turnLedOff(Led led);
    void turnAllLedsOff();
}

//namespace platform {
//    void initLed();
//    void turnLedOn();
//    void turnLedOff();
//    void toggleLed();
//}
