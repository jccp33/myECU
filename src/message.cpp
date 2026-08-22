#include "../include/utils.hpp"
#include "../include/message.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

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
    timestampMs = get_timestamp_ms();
}

Message::Message(uint32_t id, SensorId sId, const std::string& sensorName,
                 const std::string& sensorUnit, float val, bool critic,
                 float min, float max, uint64_t timeout, bool shutdownRequest,
                 float shutdownValue){
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
    timestampMs = get_timestamp_ms();
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

void Message::setTimesStamp(uint64_t timestamp){
    timestampMs = timestamp;
}

// getters
uint32_t Message::getMessageId() {
    return messageId;
}

SensorId Message::getSensorId() {
    return sensorId;
}

float Message::getRawValue() {
    return rawValue;
}

float Message::getMinValue() {
    return minValue;
}

float Message::getMaxValue() {
    return maxValue;
}

SignalStatus Message::getSignalStatus() {
    return status;
}

bool Message::getIsCritic() {
    return isCritic;
}

uint64_t Message::getTimeoutMs() const {
    return timeoutMs;
}

bool Message::getIsShutdownRequest() const {
    return isShutdownRequest;
}

float Message::getActiveValue() const {
    return activeValue;
}

uint64_t Message::getTimestamp() {
    return timestampMs;
}

const char* Message::getUnit() const {
    return unit.c_str();
}

const char* Message::getName() const {
    return name.c_str();
}

// methods
std::string Message::getMessageString() const {
    // status to string
    std::string _status = "";
    if(status == SignalStatus::VALID) _status = "VALID";
    else if(status == SignalStatus::OUT_OF_RANGE) _status = "OUT_OF_RANGE";
    else if(status == SignalStatus::TIMEOUT) _status = "TIMEOUT";
    else _status = "UNDEFINED";
    // isCritic to string
    std::string _isCritic = isCritic ? "TRUE" : "FALSE";
    // result
    std::stringstream ss;
    ss << std::left << std::setw(10) << messageId
       << std::setw(10) << static_cast<int>(sensorId) 
    << std::setw(30) << getName()
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
    << std::setw(10) << getUnit()
       << std::setw(15) << _status 
       << std::setw(10) << _isCritic
       << std::setw(15) << timestampMs;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdMessageString() const {
    // status to string
    std::string _status = "";
    if(status == SignalStatus::VALID) _status = "valido";
    else if(status == SignalStatus::OUT_OF_RANGE) _status = "fuera de rango";
    else if(status == SignalStatus::TIMEOUT) _status = "fuera de tiempo";
    else _status = "indefinido";
    // result
    std::stringstream ss;
    ss << std::left
    << std::setw(25) << getName()
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
    << std::setw(6) << getUnit()
       << std::setw(16) << _status;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdColorsMessageString() const {
    // status to string
    std::string _status = "";
    if(status == SignalStatus::VALID) _status = "valido";
    else if(status == SignalStatus::OUT_OF_RANGE) _status = "fuera de rango";
    else if(status == SignalStatus::TIMEOUT) _status = "fuera de tiempo";
    else _status = "indefinido";
    // text color
    std::string txtColor = "";
    if(status == SignalStatus::VALID) txtColor = TXT_GREEN;
    else if(status == SignalStatus::OUT_OF_RANGE) txtColor = isCritic ? TXT_RED : TXT_YELLOW;
    else if(status == SignalStatus::TIMEOUT) txtColor = isCritic ? TXT_RED : TXT_YELLOW;
    else txtColor = TXT_YELLOW;
    // result
    std::stringstream ss;
    ss << std::left
    << std::setw(24) << getName()
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
    << std::setw(6) << getUnit()
       << std::setw(10) << txtColor << _status << TXT_RESET;
    std::string result = ss.str();
    return result;
}
