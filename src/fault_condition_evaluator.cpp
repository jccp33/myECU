#include "../include/fault_condition_evaluator.hpp"

namespace {

ConditionEvaluationResult evaluateRange(
    const EvaluationRule& rule,
    float value
) {
    const bool insideRange = value >= rule.getLowerThreshold() && value <= rule.getUpperThreshold();
    return insideRange ? ConditionEvaluationResult::HEALTHY : ConditionEvaluationResult::FAULT_ACTIVE;
}

ConditionEvaluationResult evaluateTimeout(
    const EvaluationRule& rule,
    TimestampMs lastUpdateMs,
    TimestampMs nowMs
) {
    if (nowMs < lastUpdateMs) return ConditionEvaluationResult::CLOCK_ERROR;
    const TimestampMs elapsed = nowMs - lastUpdateMs;
    return elapsed > rule.getMaximumAgeMs() ? ConditionEvaluationResult::FAULT_ACTIVE : ConditionEvaluationResult::HEALTHY;
}

}

ConditionEvaluationResult evaluateFaultCondition(
    const EvaluationRule &rule,
    float value,
    TimestampMs lastUpdateMs,
    TimestampMs nowMs
){
    if(rule.validate() != RuleValidationError::NONE) return ConditionEvaluationResult::INVALID_RULE;
    switch(rule.getEvaluationType()){
        case EvaluationType::RANGE:
            return evaluateRange(rule, value);
        case EvaluationType::TIMEOUT:
            return evaluateTimeout(rule, lastUpdateMs, nowMs);
        case EvaluationType::RATE_OF_CHANGE:
            return ConditionEvaluationResult::UNSUPPORTED_EVALUATION_TYPE;
    }
    return ConditionEvaluationResult::UNSUPPORTED_EVALUATION_TYPE;
}
