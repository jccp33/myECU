#include "../include/mssgmanager.hpp"

// methods
void MessageManager::InitMessage(const InitValues &values, Message &mssg){
    mssg = Message(
        values.id,
        values.sId,
        values.value,
        values.isCritic,
        values.minValue,
        values.maxValue
    );
}

void MessageManager::UpdateMessage(uint64_t time, float value, Message &mssg){
    mssg.setTimesStamp(time);
    mssg.setRawValue(value);
}
