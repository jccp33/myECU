#include "fault_configuration.hpp"

namespace {
    bool containsDuplicateSignals(const SystemConfig& systemConfig) {
        for (std::size_t first = 0U; first < systemConfig.sensorCount; ++first) {
            for (
                std::size_t second = first + 1U;
                second < systemConfig.sensorCount;
                ++second
            ){
                if(
                    systemConfig.sensors[first].signalId == systemConfig.sensors[second].signalId
                ) {
                    return true;
                }
            }
        }
        return false;
    }
    bool containsSignal(const SystemConfig& systemConfig, const SignalId& signalId) {
        for (std::size_t i = 0U; i < systemConfig.sensorCount; ++i) {
            if (systemConfig.sensors[i].signalId == signalId) {
                return true;
            }
        }
        return false;
    }
    bool containsDuplicateRules(const EvaluationRuleSet& ruleSet) {
        for (std::size_t first = 0U; first < ruleSet.count; ++first) {
            for (
                std::size_t second = first + 1U;
                second < ruleSet.count;
                ++second
            ) {
                if(ruleSet.rules[first].getSignalId() == ruleSet.rules[second].getSignalId()){
                    return true;
                }
            }
        }
        return false;
    }
} // namespace


FaultConfigurationError buildEvaluationRuleSet(
    const SystemConfig& systemConfig,
    std::array<EvaluationRule, MAX_SENSOR_COUNT>& ruleStorage,
    EvaluationRuleSet& ruleSet
) {
    ruleSet = EvaluationRuleSet();
    if (systemConfig.sensorCount > systemConfig.sensors.size()) {
        return FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION;
    }
    if (containsDuplicateSignals(systemConfig)) {
        return FaultConfigurationError::DUPLICATE_SIGNAL;
    }
    for (std::size_t i = 0U; i < systemConfig.sensorCount; ++i) {
        const InitValues& sensor = systemConfig.sensors[i];
        EvaluationRule rule(
            sensor.signalId,
            sensor.confirmationTimeMs,
            sensor.recoveryTimeMs,
            sensor.severity,
            sensor.latching
        );
        if (!rule.isValid()) {
            return FaultConfigurationError::INVALID_RULE;
        }
        ruleStorage[i] = rule;
    }
    ruleSet = EvaluationRuleSet(
        ruleStorage.data(),
        systemConfig.sensorCount
    );
    return FaultConfigurationError::NONE;
}

FaultConfigurationError validateEvaluationRuleSet(
    const EvaluationRuleSet& ruleSet,
    const SystemConfig& systemConfig
) {
    if (systemConfig.sensorCount > systemConfig.sensors.size()) {
        return FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION;
    }
    if (containsDuplicateSignals(systemConfig)) {
        return FaultConfigurationError::DUPLICATE_SIGNAL;
    }
    if (ruleSet.count != systemConfig.sensorCount) {
        return FaultConfigurationError::INVALID_RULE;
    }
    if (ruleSet.count > 0U && ruleSet.rules == nullptr) {
        return FaultConfigurationError::INVALID_RULE;
    }
    if (containsDuplicateRules(ruleSet)) {
        return FaultConfigurationError::DUPLICATE_SIGNAL;
    }
    for (std::size_t i = 0U; i < ruleSet.count; ++i) {
        const EvaluationRule& rule = ruleSet.rules[i];
        if (!rule.isValid()) {
            return FaultConfigurationError::INVALID_RULE;
        }
        if (!containsSignal(systemConfig, rule.getSignalId())) {
            return FaultConfigurationError::INVALID_RULE;
        }
    }
    return FaultConfigurationError::NONE;
}
