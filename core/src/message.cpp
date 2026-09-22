#include "message.hpp"

// constructors
Message::Message() {
    signalId = SignalId();
    rawValue = 0.0f;
    minValue = 0.0f;
    maxValue = 0.0f;
    status = SignalStatus::UNDEFINED;
    timeoutMs = 0U;
    isShutdownRequest = false;
    isShutdownPermission = false;
    activeValue = 0.0f;
    timestampMs = 0U;
}

Message::Message(
    const SignalId &sigId,
    float val,
    float min,
    float max,
    TimestampMs timeout,
    bool shutdownRequest,
    bool shutdownPermission,
    float shutdownValue,
    TimestampMs timestamp
) {
    signalId = sigId;
    rawValue = val;
    minValue = min;
    maxValue = max;
    status = SignalStatus::VALID;
    timeoutMs = timeout;
    isShutdownRequest = shutdownRequest;
    isShutdownPermission = shutdownPermission;
    activeValue = shutdownValue;
    timestampMs = timestamp;
}

// setters
void Message::setSignalStatus(SignalStatus _status) {
    status = _status;
}

void Message::setRawValue(float value) {
    rawValue = value;
}

void Message::setTimesStamp(TimestampMs timestamp) {
    timestampMs = timestamp;
}

// getters
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

TimestampMs Message::getTimeoutMs() const {
    return timeoutMs;
}

bool Message::getIsShutdownRequest() const {
    return isShutdownRequest;
}

bool Message::getIsShutdownPermission() const {
    return isShutdownPermission;
}

float Message::getActiveValue() const {
    return activeValue;
}

TimestampMs Message::getTimestamp() const {
    return timestampMs;
}
