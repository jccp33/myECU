#include "signal_acquisition.hpp"
#include "adc.hpp"
#include <cstdint>

namespace app 
{
    namespace
    {
        constexpr float ADC_REFERENCE_VOLTAGE = 3.3F;
        constexpr float ADC_MAX_VALUE = 4095.0F;
        const SignalId TPS_SIGNAL_ID(1U, 1U, 106U, 0U);
        float adcToVoltage(std::uint16_t adcRaw)
        {
            return (static_cast<float>(adcRaw) * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
        }

    }

    void acquireSignals(SignalStore &signalStore, TimestampMs now)
    {
        const std::uint16_t adcRaw = platform::readAdc();
        const float tpsVoltage = adcToVoltage(adcRaw);
        const SignalSample tpsSample(
            TPS_SIGNAL_ID,
            tpsVoltage,
            now,
            SignalValidity::VALID
        );
        const SignalStoreResult result = signalStore.upsert(tpsSample);
        (void)result;
    }
}
