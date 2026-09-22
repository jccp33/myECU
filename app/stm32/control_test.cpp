#include "adc.hpp"
#include "time.hpp"
#include "config.hpp"
#include "control.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include "getaway.hpp"
#include "mssgmanager.hpp"
#include "signal_acquisition.hpp"
#include "led.hpp"
#include <array>
#include <cstdint>

volatile bool g_hasDegraded = false;
volatile bool g_hasCriticalActive = false;
volatile std::uint8_t g_ecuState = 0U;
volatile std::uint8_t g_tpsFaultState = 0U;
volatile float g_tpsVoltage = 0.0F;
volatile std::uint8_t g_temperatureSignalStatus = 0U;
volatile float g_temperatureEstimatedC = 0.0F;

namespace {
    const SignalId TPS_SIGNAL_ID(1U, 1U, 106U, 0U);
    const SignalId TEMPERATURE_SIGNAL_ID(1U, 1U, 104U, 0U);
    
    Message* findMessage(
        std::array<Message, MAX_SENSOR_COUNT>& messages,
        std::size_t messageCount,
        const SignalId& signalId
    ) {
        for (std::size_t index = 0U; index < messageCount; ++index) {
            if (messages[index].getSignalId() == signalId) return &messages[index];
        }
        return nullptr;
    }
    
    void refreshNominalSignals(
        const SystemConfig& config,
        std::array<Message, MAX_SENSOR_COUNT>& messages,
        const MessageManager& messageManager,
        TimestampMs now
    ) {
        for (std::size_t index = 0U; index < config.sensorCount; ++index) {
            const SignalId& id = messages[index].getSignalId();
            if (id == TPS_SIGNAL_ID || id == TEMPERATURE_SIGNAL_ID) continue;
            const float nominal = config.sensors[index].minValue
                + ((config.sensors[index].maxValue - config.sensors[index].minValue) / 2.0F);
            messageManager.UpdateMessage(now, nominal, messages[index]);
        }
    }
    
    void showState(EcuState state) {
        platform::turnAllLedsOff();
        switch (state) {
            case EcuState::INIT:
            case EcuState::SELF_TEST:
                platform::turnLedOn(platform::Led::BLUE);
                break;
            case EcuState::OPERATIONAL:
                platform::turnLedOn(platform::Led::GREEN);
                break;
            case EcuState::DEGRADED:
                platform::turnLedOn(platform::Led::YELLOW);
                break;
            case EcuState::SAFE_STATE:
                platform::turnLedOn(platform::Led::RED);
                break;
            case EcuState::SHUTDOWN_REQ:
            case EcuState::SHUTDOWN:
                break;
        }
    }

} // namespace

int main() {
    // init
    platform::initLeds();
    platform::initTime();
    platform::initAdc();
    // objects
    const SystemConfig systemConfig = getSystemConfig();
    std::array<EvaluationRule, MAX_SENSOR_COUNT> ruleStorage;
    EvaluationRuleSet ruleSet;
    const FaultConfigurationError configurationError = buildEvaluationRuleSet(
        systemConfig,
        ruleStorage,
        ruleSet
    );
    std::array<Message, MAX_SENSOR_COUNT> messages;
    const MessageManager messageManager;
    for (std::size_t index = 0U; index < systemConfig.sensorCount; ++index) {
        messages[index] = messageManager.InitMessage(systemConfig.sensors[index], 0U);
    }
    FaultManager faultManager(ruleSet.rules, ruleSet.count);
    const Gateway gateway;
    Control control;
    std::uint32_t previousTime = 0U;
    // main loop
    while (true) {
        const std::uint32_t now = platform::millis();
        if ((now - previousTime) < 100U) continue;
        previousTime = now;
        const TimestampMs cycleTime = static_cast<TimestampMs>(now);

        if (configurationError == FaultConfigurationError::NONE) {
            refreshNominalSignals(systemConfig, messages, messageManager, cycleTime);
            app::acquireSignals(
                messages,
                systemConfig.sensorCount,
                messageManager,
                cycleTime
            );
            for (std::size_t index = 0U; index < systemConfig.sensorCount; ++index) {
                gateway.validateMessage(messages[index], cycleTime);
            }
            control.processMessages(
                messages,
                systemConfig.sensorCount,
                faultManager,
                cycleTime
            );
        }

        const Message* const tps = findMessage(messages, systemConfig.sensorCount, TPS_SIGNAL_ID);
        if (tps != nullptr) g_tpsVoltage = tps->getRawValue();
        const Message* const temperature = findMessage(
            messages,
            systemConfig.sensorCount,
            TEMPERATURE_SIGNAL_ID
        );
        if (temperature != nullptr) {
            g_temperatureEstimatedC = temperature->getRawValue();
            g_temperatureSignalStatus = static_cast<std::uint8_t>(temperature->getSignalStatus());
        }

        const FaultSummary faultSummary = faultManager.getSummary();
        g_hasDegraded = faultSummary.hasDegraded;
        g_hasCriticalActive = faultSummary.hasCriticalActive;
        const FaultRecord* const tpsFaultRecord = faultManager.getRecord(TPS_SIGNAL_ID);
        if (tpsFaultRecord != nullptr) {
            g_tpsFaultState = static_cast<std::uint8_t>(tpsFaultRecord->state);
        }
        g_ecuState = static_cast<std::uint8_t>(control.getCurrentState());
        showState(control.getCurrentState());
    }
}
