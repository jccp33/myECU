#include "mssgmanager.hpp"

Message MessageManager::InitMessage(
    const InitValues &values, 
    TimestampMs timestamp
) const {
    return Message(
        values.signalId,
        values.value,
        values.minValue,
        values.maxValue,
        values.timeoutMs,
        values.isShutdownRequest,
        values.isShutdownPermission,
        values.activeValue,
        timestamp
    );
}

void MessageManager::UpdateMessage(
    TimestampMs timestamp, 
    float value, 
    Message &mssg
) const {
    mssg.setTimesStamp(timestamp);
    mssg.setRawValue(value);
}
