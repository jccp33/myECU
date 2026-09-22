#include "getaway.hpp"

bool Gateway::validateValue(float value, float min, float max) const {
    return (value>=min && value<=max);
}

GatewayResult Gateway::validateMessage(Message &mssg, TimestampMs currentTimeMs) const {
    if (currentTimeMs < mssg.getTimestamp()) {
        mssg.setSignalStatus(SignalStatus::UNDEFINED);
        return GatewayResult::CLOCK_ERROR;
    }
    if ((currentTimeMs - mssg.getTimestamp()) > mssg.getTimeoutMs()) {
        mssg.setSignalStatus(SignalStatus::TIMEOUT);
        return GatewayResult::OK;
    }
    if (mssg.getRawValue() < mssg.getMinValue() ||
        mssg.getRawValue() > mssg.getMaxValue()) {
        mssg.setSignalStatus(SignalStatus::OUT_OF_RANGE);
        return GatewayResult::OK;
    }
    mssg.setSignalStatus(SignalStatus::VALID);
    return GatewayResult::OK;
}
