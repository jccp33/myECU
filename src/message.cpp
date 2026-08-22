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
    timestampMs = get_timestamp_ms();
    unit = "";
}

Message::Message(uint32_t id, SensorId sId, float val, bool critic, float min, float max, std::string _unit, std::string _name){
    messageId = id;
    sensorId = sId;
    rawValue = val;
    minValue = min;
    maxValue = max;
    status = SignalStatus::VALID;
    isCritic = critic;
    timestampMs = get_timestamp_ms();
    unit = _unit;
    name = _name;
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

void Message::setUnit(std::string _unit){
    unit = _unit;
}

void Message::setName(std::string _name){
    name = _name;
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

uint64_t Message::getTimestamp() {
    return timestampMs;
}

std::string Message::getUnit() {
    return unit;
}

std::string Message::getName(){
    return name;
}

// methods
std::string Message::getMessageString(){
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
       << std::setw(30) << name
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(10) << unit 
       << std::setw(15) << _status 
       << std::setw(10) << _isCritic
       << std::setw(15) << timestampMs;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdMessageString(){
    // status to string
    std::string _status = "";
    if(status == SignalStatus::VALID) _status = "valido";
    else if(status == SignalStatus::OUT_OF_RANGE) _status = "fuera de rango";
    else if(status == SignalStatus::TIMEOUT) _status = "fuera de tiempo";
    else _status = "indefinido";
    // result
    std::stringstream ss;
    ss << std::left
       << std::setw(25) << name
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(6) << unit 
       << std::setw(16) << _status;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdColorsMessageString(){
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
       << std::setw(24) << name
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(6) << unit 
       << std::setw(10) << txtColor << _status << TXT_RESET;
    std::string result = ss.str();
    return result;
}
