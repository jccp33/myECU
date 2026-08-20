#ifndef GETAWAY_HPP
#define GETAWAY_HPP

#define MAXIMUM_TIME_IN_MS 500

#include "../include/message.hpp"
#include <cstdint>

class Gateway {
    private:
        bool validateValue(float value, float min, float max);
    public:
        Gateway() = default;
        // methods
        void validateMessage(Message &mssg, uint64_t currentTimeMs);
};

#endif
