#ifndef GETAWAY_HPP
#define GETAWAY_HPP

#include "../include/message.hpp"
#include <cstdint>

class Gateway {
    private:
        bool validateValue(float value, float min, float max);
    public:
        // constructor
        Gateway() = default;
        // methods
        void validateMessage(Message &mssg, uint64_t currentTimeMs);
};

#endif
