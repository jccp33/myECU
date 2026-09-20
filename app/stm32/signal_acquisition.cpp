#include "signal_acquisition.hpp"
#include "adc.hpp"
#include <cstdint>

namespace app 
{
    namespace
    {
        // Experimental diagnostic thresholds.
        // TODO: replace with limits derived from the sensor electrical specification.
        constexpr std::uint16_t ADC_MIN_VALID_RAW = 100U;
        constexpr std::uint16_t ADC_MAX_VALID_RAW = 4000U;
        constexpr float ADC_REFERENCE_VOLTAGE = 3.3F;
        constexpr float ADC_MAX_VALUE = 4095.0F;
        const SignalId TPS_SIGNAL_ID(1U, 1U, 106U, 0U);
        const SignalId TEMPERATURE_SIGNAL_ID(1U, 1U, 104U, 0U);
        const SignalId NTC_RESISTANCE_SIGNAL_ID(1U, 1U, 110U, 0U);
        constexpr float NTC_FIXED_RESISTOR_OHMS = 9970.0F;
        constexpr float NTC_NOMINAL_RESISTANCE_OHMS = 10000.0F;
        constexpr float NTC_NOMINAL_TEMPERATURE_K = 298.15F;
        constexpr float NTC_BETA_K = 3950.0F; // Provisional, not calibrated.
        
        float adcToVoltage(std::uint16_t adcRaw)
        {
            return (static_cast<float>(adcRaw) * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
        }
        
        float voltageToNtcResistance(float voltage)
        {
            if(voltage >= ADC_REFERENCE_VOLTAGE) return 0.0F;
            return (NTC_FIXED_RESISTOR_OHMS * voltage) / (ADC_REFERENCE_VOLTAGE - voltage);
        }

        // log(x) -> alternative cmath version
        constexpr float LN2 = 0.69314718056F;
        float naturalLog(float x)
        {
            if (x <= 0.0F)return 0.0F;
            int k = 0;
            while (x > 1.5F){
                x *= 0.5F;
                ++k;
            }
            while (x < 0.5F){
                x *= 2.0F;
                --k;
            }
            const float z = (x - 1.0F) / (x + 1.0F);
            const float zSquared = z * z;
            float term = z;
            float sum = term;
            for (int n = 3; n <= 11; n += 2){
                term *= zSquared;
                sum += term / static_cast<float>(n);
            }
            return (2.0F * sum) + (static_cast<float>(k) * LN2);
        }

        float ntcResistanceToTemperature(float resistance){
            if (resistance <= 0.0F) return 0.0F;
            const float inverseTemperature =
                (1.0F / NTC_NOMINAL_TEMPERATURE_K) +
                (1.0F / NTC_BETA_K) *
                naturalLog(resistance / NTC_NOMINAL_RESISTANCE_OHMS);
            const float temperatureKelvin =
                1.0F / inverseTemperature;
            return temperatureKelvin - 273.15F;
        }
        
        void acquireTps(SignalStore &signalStore, TimestampMs now){
            const std::uint16_t adcRaw = platform::readAdc(0U);
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
        
        void acquireNtcResistance(SignalStore &signalStore, TimestampMs now){
            const std::uint16_t adcRaw = platform::readAdc(1U);
            if ((adcRaw <= ADC_MIN_VALID_RAW) || (adcRaw >= ADC_MAX_VALID_RAW)){
                const SignalSample invalidSample(
                    NTC_RESISTANCE_SIGNAL_ID,
                    0.0F,
                    now,
                    SignalValidity::INVALID
                );
                const SignalStoreResult result = signalStore.upsert(invalidSample);
                (void)result;
                const SignalSample invalidTempSample(
                    TEMPERATURE_SIGNAL_ID,
                    0.0F,
                    now,
                    SignalValidity::INVALID
                );
                const SignalStoreResult tempResult = signalStore.upsert(invalidTempSample);
                (void)tempResult;
                return;
            }
            const float sensorVoltaje = adcToVoltage(adcRaw);
            const float ntcResistance = voltageToNtcResistance(sensorVoltaje);
            const float temperatureC = ntcResistanceToTemperature(ntcResistance);
            const SignalSample temperatureSample(
                TEMPERATURE_SIGNAL_ID,
                temperatureC,
                now,
                SignalValidity::VALID
            );
            const SignalStoreResult temperatureResult = signalStore.upsert(temperatureSample);
            (void)temperatureResult;
            const SignalSample tempSample(
                NTC_RESISTANCE_SIGNAL_ID,
                ntcResistance,
                now,
                SignalValidity::VALID
            );
            const SignalStoreResult result = signalStore.upsert(tempSample);
            (void)result;
        }
    }

    void acquireSignals(SignalStore &signalStore, TimestampMs now)
    {
        acquireTps(signalStore, now);
        acquireNtcResistance(signalStore, now);
    }
}
