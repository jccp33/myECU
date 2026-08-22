#include "../include/getaway.hpp"

bool Gateway::validateValue(float value, float min, float max) const {
    return (value>=min && value<=max);
}

void Gateway::validateMessage(Message &mssg, TimestampMs currentTimeMs) const {
    if ((currentTimeMs - mssg.getTimestamp()) > mssg.getTimeoutMs()) {
        mssg.setSignalStatus(SignalStatus::TIMEOUT);
        return;
    }
    bool isValid = validateValue(mssg.getRawValue(), mssg.getMinValue(), mssg.getMaxValue());
    if(isValid){
        mssg.setSignalStatus(SignalStatus::VALID);
    }else{
        mssg.setSignalStatus(SignalStatus::OUT_OF_RANGE);
    }
}
