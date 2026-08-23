#include "sensor_simulation.hpp"

#include <cstdlib>
#include <iostream>

namespace {

bool expectTrue(const char* name, bool condition) {
    if(!condition) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}

bool testTemperatureChangesGradually() {
    const float next = simulateSensorValue(SensorId::TEMP, 25.0F, 1.0F);
    return expectTrue(
        "temperature changes gradually",
        next > 25.0F && next < 26.0F
    );
}

bool testVoltageRemainsStable() {
    const float lowNoise = simulateSensorValue(SensorId::VOLTAGE, 12.5F, -1.0F);
    const float highNoise = simulateSensorValue(SensorId::VOLTAGE, 12.5F, 1.0F);
    return expectTrue(
        "voltage remains inside a small step",
        lowNoise >= 12.5F && highNoise <= 12.7F
    );
}

bool testEveryAnalogSignalRemainsOperational() {
    const SensorId sensors[] = {
        SensorId::SPEED, SensorId::RPM, SensorId::TEMP, SensorId::VOLTAGE,
        SensorId::TPS, SensorId::MAP, SensorId::MAF, SensorId::O2
    };
    for(std::size_t index = 0U; index < sizeof(sensors) / sizeof(sensors[0]); ++index) {
        float value = 0.0F;
        if(sensors[index] == SensorId::TEMP) value = 25.0F;
        if(sensors[index] == SensorId::VOLTAGE) value = 12.5F;
        for(std::size_t cycle = 0U; cycle < 1000U; ++cycle) {
            const float noise = cycle % 2U == 0U ? -1.0F : 1.0F;
            value = simulateSensorValue(sensors[index], value, noise);
        }
        if(value < -20.0F || value > 4500.0F) return expectTrue("analog values remain bounded", false);
    }
    return expectTrue("analog values remain bounded", true);
}

bool testDiscreteSignalsAreNotModified() {
    return expectTrue(
        "discrete signals are controlled by user commands",
        simulateSensorValue(SensorId::BRAKE, 1.0F, -1.0F) == 1.0F
            && simulateSensorValue(SensorId::SHUT_REQ, 0.0F, 1.0F) == 0.0F
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testTemperatureChangesGradually(),
        testVoltageRemainsStable(),
        testEveryAnalogSignalRemainsOperational(),
        testDiscreteSignalsAreNotModified()
    };
    for(std::size_t index = 0U; index < sizeof(results) / sizeof(results[0]); ++index) {
        if(!results[index]) ++failures;
    }
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
