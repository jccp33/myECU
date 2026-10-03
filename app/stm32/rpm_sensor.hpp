#pragma once

#include <cstdint>

namespace app {
    struct RpmSensorConfig
    {
        std::uint32_t pulsesPerRevolution;
        std::uint32_t maxRpm;
    };

    class RpmSensor {
        public:
            explicit RpmSensor(const RpmSensorConfig& config);
            bool calculateRpm(std::uint32_t periodUs, float &rpm) const;
        private:
            RpmSensorConfig config_;
    };
} // namespace app
