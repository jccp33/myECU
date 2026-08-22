#ifndef GETAWAY_HPP
#define GETAWAY_HPP

#include "../include/message.hpp"

class Gateway {
    private:
        bool validateValue(float value, float min, float max) const;
    public:
        // constructor
        Gateway() = default;
        // methods
        void validateMessage(Message &mssg, TimestampMs currentTimeMs) const;
};

#endif
