#include "../include/message.hpp"

// constructors
Message::Message(){
    messageId = 0;
    sensorId = SensorId::UNDEFINED;
    rawValue = 0.0f;
    minValue = 0.0f;
    maxValue = 0.0f;
    status = SignalStatus::UNDEFINED;
    isCritic = false;
    timeoutMs = 0;
    isShutdownRequest = false;
    activeValue = 0.0f;
    timestampMs = 0;
}

Message::Message(
    uint32_t id, 
    SensorId sId, 
    const std::string &sensorName,
    const std::string &sensorUnit, 
    float val, 
    bool critic,
    float min, 
    float max, 
    TimestampMs timeout, 
    bool shutdownRequest,
    float shutdownValue,
    TimestampMs timestamp
){
    messageId = id;
    sensorId = sId;
    name = sensorName;
    unit = sensorUnit;
    rawValue = val;
    minValue = min;
    maxValue = max;
    status = SignalStatus::VALID;
    isCritic = critic;
    timeoutMs = timeout;
    isShutdownRequest = shutdownRequest;
    activeValue = shutdownValue;
    timestampMs = timestamp;
}

// setters
void Message::setSignalStatus(SignalStatus _status){
    status = _status;
}

void Message::setMessageId(uint32_t id){
    messageId = id;
}

void Message::setSensorId(SensorId id){
    sensorId = id;
}

void Message::setIsCritic(bool critic){
    isCritic = critic;
}

void Message::setRawValue(float value){
    rawValue = value;
}

void Message::setMinValue(float min){
    minValue = min;
}

void Message::setMaxValue(float max){
    maxValue = max;
}

void Message::setTimesStamp(TimestampMs timestamp){
    timestampMs = timestamp;
}

// getters
uint32_t Message::getMessageId() const {
    return messageId;
}

SensorId Message::getSensorId() const {
    return sensorId;
}

float Message::getRawValue() const {
    return rawValue;
}

float Message::getMinValue() const {
    return minValue;
}

float Message::getMaxValue() const {
    return maxValue;
}

SignalStatus Message::getSignalStatus() const {
    return status;
}

bool Message::getIsCritic() const {
    return isCritic;
}

TimestampMs Message::getTimeoutMs() const {
    return timeoutMs;
}

bool Message::getIsShutdownRequest() const {
    return isShutdownRequest;
}

float Message::getActiveValue() const {
    return activeValue;
}

TimestampMs Message::getTimestamp() const {
    return timestampMs;
}

const std::string &Message::getUnit() const {
    return unit;
}

const std::string &Message::getName() const {
    return name;
}
