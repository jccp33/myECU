#include "rpm_input.hpp"
#include "rpm_sensor.hpp"
#include <cstdint>

volatile std::uint32_t g_rpmPeriodUs = 0U;
volatile std::uint32_t g_rpmPeriodCount = 0U;
volatile float g_rpm = 0.0f;
volatile std::uint32_t g_validRpmCount = 0U;
volatile std::uint32_t g_rejectedRpmCount = 0U;

int main()
{
    platform::initRpmInput();
    const app::RpmSensorConfig rpmConfig{
        1U,     // pulsesPerRevolution - provisional
        7000U   // maxRpm
    };
    const app::RpmSensor rpmSensor{rpmConfig};
    while (true) {
        std::uint32_t periodUs = 0U;
        if (platform::readRpmPeriodUs(periodUs)) {
            g_rpmPeriodUs = periodUs;
            ++g_rpmPeriodCount;
            float rpm = 0.0f;
            if (rpmSensor.calculateRpm(periodUs, rpm)) {
                g_rpm = rpm;
                ++g_validRpmCount;
            } else {
                ++g_rejectedRpmCount;
            }
        }
    }
}
