#include "rpm_sensor.hpp"

namespace app {
    RpmSensor::RpmSensor(const RpmSensorConfig& config) : config_(config) {}

    bool RpmSensor::calculateRpm(std::uint32_t periodUs, float &rpm) const {
        if (periodUs == 0U) {
            return false;
        }
        if (config_.pulsesPerRevolution == 0U) {
            return false;
        }
        if (config_.maxRpm == 0U) {
            return false;
        }
        const std::uint64_t maxPulseFrequency = static_cast<std::uint64_t>(config_.maxRpm) *
            static_cast<std::uint64_t>(config_.pulsesPerRevolution);
        const std::uint64_t minimumPeriodUs = 60000000ULL / maxPulseFrequency;
        if (static_cast<std::uint64_t>(periodUs) < minimumPeriodUs) {
            return false;
        }
        const std::uint64_t denominator = static_cast<std::uint64_t>(periodUs) *
            static_cast<std::uint64_t>(config_.pulsesPerRevolution);
        rpm = 60000000.0f / static_cast<float>(denominator);
        return true;
    }
} // namespace app
