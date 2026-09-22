#include "adc.hpp"
#include "led.hpp"
#include "time.hpp"
#include "config.hpp"
#include "control.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include "getaway.hpp"
#include "mssgmanager.hpp"
#include "signal_acquisition.hpp"
#include <array>
#include <cstdint>

namespace {
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
    const TimestampMs initialTime = static_cast<TimestampMs>(platform::millis());
    for (std::size_t index = 0U; index < systemConfig.sensorCount; ++index) {
        messages[index] = messageManager.InitMessage(systemConfig.sensors[index], initialTime);
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

        showState(control.getCurrentState());
    }
}
