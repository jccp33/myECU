#ifndef SENSOR_SIMULATION_HPP
#define SENSOR_SIMULATION_HPP

#include "data_types.hpp"

float simulateSensorValue(
    SensorId sensorId,
    float currentValue,
    float normalizedNoise
);

#endif
