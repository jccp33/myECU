#include "../include/sensor_simulation.hpp"

namespace {

float clampValue(float value, float minimum, float maximum) {
    if(value < minimum) return minimum;
    if(value > maximum) return maximum;
    return value;
}

float evolveValue(
    float currentValue,
    float targetValue,
    float response,
    float noiseAmplitude,
    float minimum,
    float maximum,
    float normalizedNoise
) {
    const float noise = clampValue(normalizedNoise, -1.0F, 1.0F);
    const float tendency = (targetValue - currentValue) * response;
    return clampValue(
        currentValue + tendency + noise * noiseAmplitude,
        minimum,
        maximum
    );
}

}  // namespace

float simulateSensorValue(
    SensorId sensorId,
    float currentValue,
    float normalizedNoise
) {
    switch(sensorId) {
        case SensorId::SPEED:
            return evolveValue(currentValue, 60.0F, 0.03F, 2.0F, 0.0F, 130.0F, normalizedNoise);
        case SensorId::RPM:
            return evolveValue(currentValue, 1800.0F, 0.10F, 120.0F, 0.0F, 4500.0F, normalizedNoise);
        case SensorId::TEMP:
            return evolveValue(currentValue, 90.0F, 0.01F, 0.15F, -20.0F, 110.0F, normalizedNoise);
        case SensorId::VOLTAGE:
            return evolveValue(currentValue, 13.8F, 0.08F, 0.08F, 11.5F, 14.8F, normalizedNoise);
        case SensorId::TPS:
            return evolveValue(currentValue, 1.5F, 0.08F, 0.08F, 0.5F, 4.5F, normalizedNoise);
        case SensorId::MAP:
            return evolveValue(currentValue, 2.3F, 0.08F, 0.08F, 0.5F, 4.5F, normalizedNoise);
        case SensorId::MAF:
            return evolveValue(currentValue, 25.0F, 0.08F, 1.0F, 2.0F, 100.0F, normalizedNoise);
        case SensorId::O2:
            return evolveValue(currentValue, 0.45F, 0.15F, 0.08F, 0.1F, 0.9F, normalizedNoise);
        case SensorId::BRAKE:
        case SensorId::SHUT_REQ:
        case SensorId::UNDEFINED:
            return currentValue;
    }
    return currentValue;
}
