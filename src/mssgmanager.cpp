#include "../include/mssgmanager.hpp"

Message MessageManager::InitMessage(const InitValues &values){
    Message mssg = Message(
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
        values.activeValue
    );
    return mssg;
}

void MessageManager::UpdateMessage(uint64_t time, float value, Message &mssg){
    mssg.setTimesStamp(time);
    mssg.setRawValue(value);
}
