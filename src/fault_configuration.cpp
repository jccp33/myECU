#include "../include/fault_configuration.hpp"

namespace {

constexpr EvaluationRule createRangeRule(
    std::uint16_t id,
    float lower,
    float upper,
    FaultSeverity severity,
    FaultLatching latching = FaultLatching::RECOVERABLE
) {
    return EvaluationRule(
        SignalId(1U, 1U, id, 0U),
        EvaluationType::RANGE,
        lower,
        upper,
        200U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        severity,
        FaultType::SENSOR,
        latching
    );
}

constexpr EvaluationRule createTimeoutRule(
    std::uint16_t id,
    FaultSeverity severity
) {
    return EvaluationRule(
        SignalId(1U, 1U, id, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        200U,
        500U,
        500U,
        SignalError::TIMEOUT,
        severity,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );
}

const EvaluationRule EVALUATION_RULES[] = {
    createRangeRule(100U, 0.0F, 1.0F, FaultSeverity::WARNING),
    createTimeoutRule(100U, FaultSeverity::WARNING),
    createRangeRule(101U, 0.0F, 1.0F, FaultSeverity::WARNING),
    createTimeoutRule(101U, FaultSeverity::WARNING),
    createRangeRule(102U, 0.0F, 220.0F, FaultSeverity::DEGRADED),
    createTimeoutRule(102U, FaultSeverity::DEGRADED),
    createRangeRule(103U, 0.0F, 7000.0F, FaultSeverity::CRITICAL),
    createTimeoutRule(103U, FaultSeverity::CRITICAL),
    createRangeRule(104U, -20.0F, 130.0F, FaultSeverity::CRITICAL),
    createTimeoutRule(104U, FaultSeverity::CRITICAL),
    createRangeRule(
        105U, 8.0F, 16.0F, FaultSeverity::CRITICAL, FaultLatching::LATCHED
    ),
    createTimeoutRule(105U, FaultSeverity::CRITICAL),
    createRangeRule(106U, 0.5F, 4.8F, FaultSeverity::DEGRADED),
    createTimeoutRule(106U, FaultSeverity::DEGRADED),
    createRangeRule(107U, 0.5F, 4.7F, FaultSeverity::DEGRADED),
    createTimeoutRule(107U, FaultSeverity::DEGRADED),
    createRangeRule(108U, 2.0F, 120.0F, FaultSeverity::DEGRADED),
    createTimeoutRule(108U, FaultSeverity::DEGRADED),
    createRangeRule(109U, 0.1F, 0.9F, FaultSeverity::DEGRADED),
    createTimeoutRule(109U, FaultSeverity::DEGRADED)
};

constexpr std::size_t EVALUATION_RULE_COUNT = sizeof(EVALUATION_RULES) / sizeof(EVALUATION_RULES[0]);

static_assert(
    EVALUATION_RULE_COUNT <= MAX_EVALUATION_RULE_COUNT,
    "Configured evaluation rules exceed FaultManager capacity"
);

bool containsSignal(const SystemConfig &systemConfig, const SignalId &signalId) {
    for ( std::size_t i = 0U; i < systemConfig.sensorCount; i++) {
        if (systemConfig.sensors[i].signalId == signalId) return true;
    }
    return false;
}

bool containsDuplicateSignals(const SystemConfig& systemConfig) {
    for (std::size_t first = 0U; first < systemConfig.sensorCount; first++) {
        for (std::size_t second = first + 1U; second < systemConfig.sensorCount; second++) {
            if (systemConfig.sensors[first].signalId == systemConfig.sensors[second].signalId) {
                return true;
            }
        }
    }
    return false;
}

std::size_t countRulesForSignal(const EvaluationRuleSet& ruleSet, const SignalId& signalId) {
    std::size_t count = 0U;
    for (std::size_t index = 0U; index < ruleSet.count; ++index) {
        if (ruleSet.rules[index].getSignal() == signalId) count++;
    }
    return count;
}

}

EvaluationRuleSet getEvaluationRuleSet() {
    return EvaluationRuleSet(
        EVALUATION_RULES,
        EVALUATION_RULE_COUNT
    );
}

FaultConfigurationError validateEvaluationRuleSet(const EvaluationRuleSet &ruleSet, const SystemConfig &systemConfig) {
    if (ruleSet.count > MAX_EVALUATION_RULE_COUNT) {
        return FaultConfigurationError::TOO_MANY_RULES;
    }
    if (ruleSet.count > 0U && ruleSet.rules == nullptr) {
        return FaultConfigurationError::NULL_RULES;
    }
    if (systemConfig.sensorCount > systemConfig.sensors.size()) {
        return FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION;
    }
    if (containsDuplicateSignals(systemConfig)) {
        return FaultConfigurationError::DUPLICATE_SIGNAL;
    }
    for (std::size_t index = 0U; index < ruleSet.count; ++index) {
        const EvaluationRule& rule = ruleSet.rules[index];
        if (rule.validate() != RuleValidationError::NONE) {
            return FaultConfigurationError::INVALID_RULE;
        }
        if (!containsSignal(systemConfig, rule.getSignal())) {
            return FaultConfigurationError::UNKNOWN_SIGNAL;
        }
        if (countRulesForSignal(ruleSet, rule.getSignal()) > MAX_RULES_PER_SENSOR) {
            return FaultConfigurationError::TOO_MANY_RULES_FOR_SIGNAL;
        }
    }
    return FaultConfigurationError::NONE;
}
