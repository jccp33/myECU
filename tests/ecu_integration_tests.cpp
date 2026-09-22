#include "config.hpp"
#include "control.hpp"
#include "fault_configuration.hpp"
#include "getaway.hpp"
#include "mssgmanager.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

namespace {
bool expectState(const char* name, const Control& control, EcuState expected) {
    if (control.getCurrentState() != expected) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}

std::size_t findIndex(const SystemConfig& config, std::uint16_t id) {
    for (std::size_t i = 0U; i < config.sensorCount; ++i) {
        if (config.sensors[i].signalId.id == id) return i;
    }
    return config.sensorCount;
}

void initialize(const SystemConfig& config, std::array<Message, MAX_SENSOR_COUNT>& messages,
    const MessageManager& manager, TimestampMs now) {
    for (std::size_t i = 0U; i < config.sensorCount; ++i) {
        messages[i] = manager.InitMessage(config.sensors[i], now);
        manager.UpdateMessage(
            now,
            config.sensors[i].minValue
                + ((config.sensors[i].maxValue - config.sensors[i].minValue) / 2.0F),
            messages[i]
        );
    }
}

void validate(std::array<Message, MAX_SENSOR_COUNT>& messages, std::size_t count,
    const Gateway& gateway, TimestampMs now) {
    for (std::size_t i = 0U; i < count; ++i) gateway.validateMessage(messages[i], now);
}
} // namespace

int main() {
    const SystemConfig config = getSystemConfig();
    std::array<EvaluationRule, MAX_SENSOR_COUNT> ruleStorage;
    EvaluationRuleSet ruleSet;
    if (buildEvaluationRuleSet(config, ruleStorage, ruleSet) != FaultConfigurationError::NONE) {
        return EXIT_FAILURE;
    }
    std::array<Message, MAX_SENSOR_COUNT> messages;
    const MessageManager messageManager;
    const Gateway gateway;
    initialize(config, messages, messageManager, 1000U);
    validate(messages, config.sensorCount, gateway, 1000U);
    FaultManager faultManager(ruleSet.rules, ruleSet.count);
    Control control;
    control.processMessages(messages, config.sensorCount, faultManager, 1000U);
    control.processMessages(messages, config.sensorCount, faultManager, 1001U);
    bool passed = expectState("nominal pipeline reaches OPERATIONAL", control, EcuState::OPERATIONAL);

    const std::size_t speed = findIndex(config, 102U);
    messageManager.UpdateMessage(1100U, 300.0F, messages[speed]);
    validate(messages, config.sensorCount, gateway, 1100U);
    control.processMessages(messages, config.sensorCount, faultManager, 1100U);
    validate(messages, config.sensorCount, gateway, 1300U);
    control.processMessages(messages, config.sensorCount, faultManager, 1300U);
    passed = expectState("confirmed speed fault reaches DEGRADED", control, EcuState::DEGRADED) && passed;

    messageManager.UpdateMessage(1400U, 100.0F, messages[speed]);
    validate(messages, config.sensorCount, gateway, 1400U);
    control.processMessages(messages, config.sensorCount, faultManager, 1400U);
    validate(messages, config.sensorCount, gateway, 1900U);
    control.processMessages(messages, config.sensorCount, faultManager, 1900U);
    passed = expectState("healthy signal completes recovery", control, EcuState::OPERATIONAL) && passed;
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
