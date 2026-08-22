#include "../include/mssgmanager.hpp"

Message MessageManager::InitMessage(
    const InitValues &values, 
    TimestampMs timestamp
) const {
    return Message(
        values.id,
        values.sId,
        values.name,
        values.unit,
        values.value,
        values.isCritic,
        values.minValue,
        values.maxValue,
        values.timeoutMs,
        values.isShutdownRequest,
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
