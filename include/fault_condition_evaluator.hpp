#ifndef FAULT_CONDITION_EVALUATOR_HPP
#define FAULT_CONDITION_EVALUATOR_HPP

#include "evaluation_rule.hpp"

enum class ConditionEvaluationResult : std::uint8_t {
    HEALTHY = 0,
    FAULT_ACTIVE,
    INVALID_RULE,
    CLOCK_ERROR,
    UNSUPPORTED_EVALUATION_TYPE
};

ConditionEvaluationResult evaluateFaultCondition(
    const EvaluationRule &rule,
    float value,
    TimestampMs lastUpdateMs,
    TimestampMs nowMs
);

#endif
