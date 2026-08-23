#ifndef MESSAGEMANAGER_HPP
#define MESSAGEMANAGER_HPP

#include "message.hpp"

class MessageManager {
    public:
        Message InitMessage(const InitValues &values, TimestampMs timestamp) const;
        void UpdateMessage(TimestampMs timestamp, float value, Message &mssg) const;
};

#endif
