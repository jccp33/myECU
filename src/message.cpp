#include "message.hpp"

// constructors
Message::Message() {
    messageId = 0;
    sensorId = SensorId::UNDEFINED;
    signalId = SignalId();
    rawValue = 0.0f;
    minValue = 0.0f;
    maxValue = 0.0f;
    status = SignalStatus::UNDEFINED;
    severity = FaultSeverity::NONE;
    timeoutMs = 0U;
    isShutdownRequest = false;
    activeValue = 0.0f;
    timestampMs = 0U;
}

Message::Message(
    uint32_t id,
    SensorId sId,
    const SignalId &sigId,
    float val,
    FaultSeverity _severity,
    float min,
    float max,
    TimestampMs timeout,
    bool shutdownRequest,
    float shutdownValue,
    TimestampMs timestamp
) {
    messageId = id;
    sensorId = sId;
    signalId = sigId;
    rawValue = val;
    minValue = min;
    maxValue = max;
    status = SignalStatus::VALID;
    severity = _severity;
    timeoutMs = timeout;
    isShutdownRequest = shutdownRequest;
    activeValue = shutdownValue;
    timestampMs = timestamp;
}


// setters

void Message::setSignalStatus(SignalStatus _status) {
    status = _status;
}

void Message::setMessageId(uint32_t id) {
    messageId = id;
}

void Message::setSensorId(SensorId id) {
    sensorId = id;
}

void Message::setSignalId(const SignalId &id) {
    signalId = id;
}

void Message::setSeverity(FaultSeverity _severity) {
    severity = _severity;
}

void Message::setRawValue(float value) {
    rawValue = value;
}

void Message::setMinValue(float min) {
    minValue = min;
}

void Message::setMaxValue(float max) {
    maxValue = max;
}

void Message::setTimesStamp(TimestampMs timestamp) {
    timestampMs = timestamp;
}

// getters
uint32_t Message::getMessageId() const {
    return messageId;
}

SensorId Message::getSensorId() const {
    return sensorId;
}

const SignalId &Message::getSignalId() const {
    return signalId;
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

FaultSeverity Message::getSeverity() const {
    return severity;
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
