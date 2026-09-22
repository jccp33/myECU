
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
