#ifndef MESSAGEMANAGER_HPP
#define MESSAGEMANAGER_HPP

#include "../include/message.hpp"
#include <string>
#include <vector>

struct InitValues {
    uint32_t id;      // message id
    SensorId sId;     // sensor id
    std::string name;
    std::string unit;
    float value;      // value
    float minValue; 
    float maxValue;
    bool isCritic;
    uint64_t timeoutMs;
    bool isShutdownRequest;
    float activeValue;
};

struct SystemConfig {
    std::vector<InitValues> sensors;
    std::size_t maxInvalidSignals;
};

class MessageManager {
    public:
        Message InitMessage(const InitValues &values, TimestampMs timestamp) const;
        void UpdateMessage(TimestampMs timestamp, float value, Message &mssg) const;
};

#endif
