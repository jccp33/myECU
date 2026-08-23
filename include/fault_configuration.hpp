#ifndef FAULT_CONFIGURATION_HPP
#define FAULT_CONFIGURATION_HPP

#include "data_types.hpp"
#include "evaluation_rule.hpp"
#include <cstddef>
#include <cstdint>

constexpr std::size_t MAX_RULES_PER_SENSOR = 3U;
constexpr std::size_t MAX_EVALUATION_RULE_COUNT = MAX_SENSOR_COUNT * MAX_RULES_PER_SENSOR;

struct EvaluationRuleSet {
    const EvaluationRule *rules;
    std::size_t count;
    constexpr EvaluationRuleSet(
        const EvaluationRule *confRules = nullptr,
        std::size_t confCount = 0U
    ) : 
        rules(confRules),
        count(confCount) {}
};

enum class FaultConfigurationError : std::uint8_t {
    NONE = 0,
    NULL_RULES,
    TOO_MANY_RULES,
    INVALID_RULE,
    INVALID_SYSTEM_CONFIGURATION,
    UNKNOWN_SIGNAL,
    TOO_MANY_RULES_FOR_SIGNAL,
    DUPLICATE_SIGNAL
};

EvaluationRuleSet getEvaluationRuleSet();

FaultConfigurationError validateEvaluationRuleSet(
    const EvaluationRuleSet &ruleSet,
    const SystemConfig &systemConfig
);

#endif
