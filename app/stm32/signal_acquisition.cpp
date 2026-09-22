#include "signal_acquisition.hpp"
#include "adc.hpp"
#include <cstdint>

namespace app {
    namespace {
        constexpr std::uint16_t ADC_MIN_VALID_RAW = 100U;
        constexpr std::uint16_t ADC_MAX_VALID_RAW = 4000U;
        constexpr float ADC_REFERENCE_VOLTAGE = 3.3F;
        constexpr float ADC_MAX_VALUE = 4095.0F;
        const SignalId TPS_SIGNAL_ID(1U, 1U, 106U, 0U);
        const SignalId TEMPERATURE_SIGNAL_ID(1U, 1U, 104U, 0U);
        constexpr float NTC_FIXED_RESISTOR_OHMS = 9970.0F;
        constexpr float NTC_NOMINAL_RESISTANCE_OHMS = 10000.0F;
        constexpr float NTC_NOMINAL_TEMPERATURE_K = 298.15F;
        constexpr float NTC_BETA_K = 3950.0F;
        constexpr float LN2 = 0.69314718056F;
        
        Message *findMessage(
            std::array<Message, MAX_SENSOR_COUNT> &messages,
            std::size_t messageCount,
            const SignalId &signalId
        ) {
            if (messageCount > messages.size()) return nullptr;
            for (std::size_t index = 0U; index < messageCount; ++index) {
                if (messages[index].getSignalId() == signalId) return &messages[index];
            }
            return nullptr;
        }
        
        float adcToVoltage(std::uint16_t adcRaw) {
            return (static_cast<float>(adcRaw) * ADC_REFERENCE_VOLTAGE) / ADC_MAX_VALUE;
        }
        
        float voltageToNtcResistance(float voltage) {
            if (voltage >= ADC_REFERENCE_VOLTAGE) return 0.0F;
            return (NTC_FIXED_RESISTOR_OHMS * voltage) / (ADC_REFERENCE_VOLTAGE - voltage);
        }
        
        float naturalLog(float value) {
            if (value <= 0.0F) return 0.0F;
            int exponent = 0;
            while (value > 1.5F) {
                value *= 0.5F;
                ++exponent;
            }
            while (value < 0.5F) {
                value *= 2.0F;
                --exponent;
            }
            const float z = (value - 1.0F) / (value + 1.0F);
            const float zSquared = z * z;
            float term = z;
            float sum = term;
            for (int divisor = 3; divisor <= 11; divisor += 2) {
                term *= zSquared;
                sum += term / static_cast<float>(divisor);
            }
            return (2.0F * sum) + (static_cast<float>(exponent) * LN2);
        }
        
        float ntcResistanceToTemperature(float resistance) {
            if (resistance <= 0.0F) return 0.0F;
            const float inverseTemperature = (1.0F / NTC_NOMINAL_TEMPERATURE_K) + 
                (1.0F / NTC_BETA_K) * naturalLog(resistance / NTC_NOMINAL_RESISTANCE_OHMS);
            return (1.0F / inverseTemperature) - 273.15F;
        }
        
        void acquireTps(
            std::array<Message, MAX_SENSOR_COUNT> &messages,
            std::size_t messageCount,
            const MessageManager &messageManager,
            TimestampMs now
        ) {
            Message* const message = findMessage(messages, messageCount, TPS_SIGNAL_ID);
            if (message == nullptr) return;
            messageManager.UpdateMessage(now, adcToVoltage(platform::readAdc(0U)), *message);
        }
        
        void acquireTemperature(
            std::array<Message, MAX_SENSOR_COUNT> &messages,
            std::size_t messageCount,
            const MessageManager& messageManager,
            TimestampMs now
        ) {
            const std::uint16_t adcRaw = platform::readAdc(1U);
            if (adcRaw <= ADC_MIN_VALID_RAW || adcRaw >= ADC_MAX_VALID_RAW) return;
            Message* const message = findMessage(messages, messageCount, TEMPERATURE_SIGNAL_ID);
            if (message == nullptr) return;
            const float voltage = adcToVoltage(adcRaw);
            const float resistance = voltageToNtcResistance(voltage);
            messageManager.UpdateMessage(now, ntcResistanceToTemperature(resistance), *message);
        }
    } // namespace

    void acquireSignals(
        std::array<Message, MAX_SENSOR_COUNT>& messages,
        std::size_t messageCount,
        const MessageManager& messageManager,
        TimestampMs now
    ) {
        acquireTps(messages, messageCount, messageManager, now);
        acquireTemperature(messages, messageCount, messageManager, now);
    }

} // namespace app
