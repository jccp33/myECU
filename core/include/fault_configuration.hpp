#ifndef FAULT_CONFIGURATION_HPP
#define FAULT_CONFIGURATION_HPP

#include "data_types.hpp"
#include "evaluation_rule.hpp"
#include <array>
#include <cstddef>
#include <cstdint>


struct EvaluationRuleSet {
    const EvaluationRule* rules;
    std::size_t count;
    constexpr EvaluationRuleSet(
        const EvaluationRule* confRules = nullptr,
        std::size_t confCount = 0U
    ) :
        rules(confRules),
        count(confCount) {
    }
};

enum class FaultConfigurationError : std::uint8_t {
    NONE = 0,
    INVALID_SYSTEM_CONFIGURATION,
    DUPLICATE_SIGNAL,
    INVALID_RULE
};

FaultConfigurationError buildEvaluationRuleSet(
    const SystemConfig& systemConfig,
    std::array<EvaluationRule, MAX_SENSOR_COUNT>& ruleStorage,
    EvaluationRuleSet& ruleSet
);

FaultConfigurationError validateEvaluationRuleSet(
    const EvaluationRuleSet& ruleSet,
    const SystemConfig& systemConfig
);

#endif
