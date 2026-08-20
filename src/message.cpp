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

Message::Message(uint32_t id, SensorId sId, float val, bool critic, float min, float max, std::string _unit){
    messageId = id;
    sensorId = sId;
    rawValue = val;
    minValue = min;
    maxValue = max;
    status = SignalStatus::VALID;
    isCritic = critic;
    timestampMs = get_timestamp_ms();
    unit = _unit;
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

// methods
std::string Message::getMessageString(){
    // sensor to string
    std::string sensorName = "";
    if(sensorId == SensorId::SHUT_REQ) sensorName = "SHUTDOWN_REQUEST";
    else if(sensorId == SensorId::SPEED) sensorName = "SPEED";
    else if(sensorId == SensorId::RPM) sensorName = "RPM";
    else if(sensorId == SensorId::TEMP) sensorName = "TEMPERATURE";
    else if(sensorId == SensorId::VOLTAGE) sensorName = "VOLTAGE";
    else if(sensorId == SensorId::BRAKE) sensorName = "BRAKE";
    else sensorName = "UNDEFINED";
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
    ss << std::left << std::setw(8) << messageId
       << std::setw(8) << static_cast<int>(sensorId) 
       << std::setw(18) << sensorName
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(6) << unit 
       << std::setw(15) << _status 
       << std::setw(8) << _isCritic
       << std::setw(15) << timestampMs;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdMessageString(){
    // sensor to string
    std::string sensorName = "";
    if(sensorId == SensorId::SHUT_REQ) sensorName = "Solicitud de Apagado";
    else if(sensorId == SensorId::SPEED) sensorName = "Velocidad";
    else if(sensorId == SensorId::RPM) sensorName = "RPM";
    else if(sensorId == SensorId::TEMP) sensorName = "Temperatura";
    else if(sensorId == SensorId::VOLTAGE) sensorName = "Voltage";
    else if(sensorId == SensorId::BRAKE) sensorName = "Freno";
    else sensorName = "Indefinido";
    // status to string
    std::string _status = "";
    if(status == SignalStatus::VALID) _status = "valido";
    else if(status == SignalStatus::OUT_OF_RANGE) _status = "fuera de rango";
    else if(status == SignalStatus::TIMEOUT) _status = "fuera de tiempo";
    else _status = "indefinido";
    // result
    std::stringstream ss;
    ss << std::left
       << std::setw(25) << sensorName
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(6) << unit 
       << std::setw(16) << _status;
    std::string result = ss.str();
    return result;
}

std::string Message::getStdColorsMessageString(){
    // sensor to string
    std::string sensorName = "";
    if(sensorId == SensorId::SHUT_REQ) sensorName = "Solicitud de Apagado";
    else if(sensorId == SensorId::SPEED) sensorName = "Velocidad";
    else if(sensorId == SensorId::RPM) sensorName = "RPM";
    else if(sensorId == SensorId::TEMP) sensorName = "Temperatura";
    else if(sensorId == SensorId::VOLTAGE) sensorName = "Voltage";
    else if(sensorId == SensorId::BRAKE) sensorName = "Freno";
    else sensorName = "Indefinido";
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
       << std::setw(24) << sensorName
       << std::setw(10) << std::fixed << std::setprecision(2) << rawValue 
       << std::setw(6) << unit 
       << std::setw(10) << txtColor << _status << TXT_RESET;
    std::string result = ss.str();
    return result;
}
