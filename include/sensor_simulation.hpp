#ifndef SENSOR_SIMULATION_HPP
#define SENSOR_SIMULATION_HPP

#include "data_types.hpp"

enum class SensorId : uint8_t {
    SHUT_REQ,
    SPEED,
    RPM,
    TEMP,
    VOLTAGE,
    BRAKE,
    TPS,
    MAP,
    MAF,
    O2,
    UNDEFINED
};

SensorId getSensorId(const SignalId& signalId);

float simulateSensorValue(
    SensorId sensorId,
    float currentValue,
    float normalizedNoise
);

#endif
