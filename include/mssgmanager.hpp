#ifndef MESSAGEMANAGER_HPP
#define MESSAGEMANAGER_HPP

#include "../include/message.hpp"

struct InitValues {
    uint32_t id;      // message id
    SensorId sId;     // sensor id
    float value;      // value
    float minValue; 
    float maxValue;
    bool isCritic;
    std::string unit;
};

class MessageManager {
    public:
        // methods
        void InitMessage(const InitValues &values, Message &mssg);
        void UpdateMessage(uint64_t time, float value, Message &mssg);
};

#endif
