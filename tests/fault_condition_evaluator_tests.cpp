#include "fault_condition_evaluator.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {

EvaluationRule createRangeRule(
    float lower,
    float upper,
    SignalError error = SignalError::OUT_OF_RANGE,
    FaultSeverity severity = FaultSeverity::CRITICAL
) {
    return EvaluationRule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        lower,
        upper,
        200U,
        500U,
        0U,
        error,
        severity,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );
}

EvaluationRule createTimeoutRule(
    TimestampMs maximumAgeMs,
    SignalError error = SignalError::TIMEOUT,
    FaultSeverity severity = FaultSeverity::DEGRADED
) {
    return EvaluationRule(
        SignalId(1U, 2U, 101U, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        maximumAgeMs,
        error,
        severity,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );
}

bool expectResult(
    const char* testName,
    ConditionEvaluationResult actual,
    ConditionEvaluationResult expected
) {
    if (actual != expected) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testRangeValueInsideIsHealthy() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    return expectResult(
        "range value inside limits is healthy",
        evaluateFaultCondition(rule, 50.0F, 0U, 0U),
        ConditionEvaluationResult::HEALTHY
    );
}

bool testRangeLowerBoundaryIsHealthy() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    return expectResult(
        "range lower boundary is inclusive",
        evaluateFaultCondition(rule, 0.0F, 0U, 0U),
        ConditionEvaluationResult::HEALTHY
    );
}

bool testRangeUpperBoundaryIsHealthy() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    return expectResult(
        "range upper boundary is inclusive",
        evaluateFaultCondition(rule, 100.0F, 0U, 0U),
        ConditionEvaluationResult::HEALTHY
    );
}

bool testRangeBelowMinimumActivatesFault() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    return expectResult(
        "range value below minimum activates fault",
        evaluateFaultCondition(rule, -1.0F, 0U, 0U),
        ConditionEvaluationResult::FAULT_ACTIVE
    );
}

bool testRangeAboveMaximumActivatesFault() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    return expectResult(
        "range value above maximum activates fault",
        evaluateFaultCondition(rule, 101.0F, 0U, 0U),
        ConditionEvaluationResult::FAULT_ACTIVE
    );
}

bool testRangeNaNActivatesFault() {
    const EvaluationRule rule = createRangeRule(0.0F, 100.0F);
    const float notANumber = std::numeric_limits<float>::quiet_NaN();
    return expectResult(
        "NaN range value activates fault",
        evaluateFaultCondition(rule, notANumber, 0U, 0U),
        ConditionEvaluationResult::FAULT_ACTIVE
    );
}

bool testInvalidRangeRuleIsRejected() {
    const EvaluationRule rule = createRangeRule(100.0F, 0.0F);
    return expectResult(
        "invalid range rule is rejected before evaluation",
        evaluateFaultCondition(rule, 50.0F, 0U, 0U),
        ConditionEvaluationResult::INVALID_RULE
    );
}

bool testTimeoutBeforeMaximumAgeIsHealthy() {
    const EvaluationRule rule = createTimeoutRule(500U);
    return expectResult(
        "signal before maximum age is healthy",
        evaluateFaultCondition(rule, 0.0F, 1000U, 1499U),
        ConditionEvaluationResult::HEALTHY
    );
}

bool testTimeoutExactBoundaryIsHealthy() {
    const EvaluationRule rule = createTimeoutRule(500U);
    return expectResult(
        "timeout boundary remains healthy",
        evaluateFaultCondition(rule, 0.0F, 1000U, 1500U),
        ConditionEvaluationResult::HEALTHY
    );
}

bool testTimeoutAfterMaximumAgeActivatesFault() {
    const EvaluationRule rule = createTimeoutRule(500U);
    return expectResult(
        "signal after maximum age activates timeout fault",
        evaluateFaultCondition(rule, 0.0F, 1000U, 1501U),
        ConditionEvaluationResult::FAULT_ACTIVE
    );
}

bool testTimeoutDetectsClockRegression() {
    const EvaluationRule rule = createTimeoutRule(500U);
    return expectResult(
        "timeout evaluation detects clock regression",
        evaluateFaultCondition(rule, 0.0F, 1000U, 999U),
        ConditionEvaluationResult::CLOCK_ERROR
    );
}

bool testZeroMaximumAgeRuleIsRejected() {
    const EvaluationRule rule = createTimeoutRule(0U);
    return expectResult(
        "zero maximum age timeout rule is rejected",
        evaluateFaultCondition(rule, 0.0F, 1000U, 1001U),
        ConditionEvaluationResult::INVALID_RULE
    );
}

bool testMismatchedTimeoutErrorIsRejected() {
    const EvaluationRule rule = createTimeoutRule(
        500U,
        SignalError::OUT_OF_RANGE
    );
    return expectResult(
        "timeout rule with mismatched error is rejected",
        evaluateFaultCondition(rule, 0.0F, 1000U, 1501U),
        ConditionEvaluationResult::INVALID_RULE
    );
}

bool testRateOfChangeRuleIsRejectedBeforeEvaluation() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 102U, 0U),
        EvaluationType::RATE_OF_CHANGE,
        0.0F,
        100.0F,
        100U,
        500U,
        0U,
        SignalError::RATE_OF_CHANGE,
        FaultSeverity::WARNING,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );
    return expectResult(
        "unsupported rate rule is rejected during validation",
        evaluateFaultCondition(rule, 50.0F, 0U, 0U),
        ConditionEvaluationResult::INVALID_RULE
    );
}

bool testResultEnumHasFixedWidthStorage() {
    const bool fixedWidth = std::is_same<
        std::underlying_type<ConditionEvaluationResult>::type,
        std::uint8_t
    >::value;
    return expectResult(
        "condition result enum uses uint8_t storage",
        fixedWidth
            ? ConditionEvaluationResult::HEALTHY
            : ConditionEvaluationResult::INVALID_RULE,
        ConditionEvaluationResult::HEALTHY
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testRangeValueInsideIsHealthy(),
        testRangeLowerBoundaryIsHealthy(),
        testRangeUpperBoundaryIsHealthy(),
        testRangeBelowMinimumActivatesFault(),
        testRangeAboveMaximumActivatesFault(),
        testRangeNaNActivatesFault(),
        testInvalidRangeRuleIsRejected(),
        testTimeoutBeforeMaximumAgeIsHealthy(),
        testTimeoutExactBoundaryIsHealthy(),
        testTimeoutAfterMaximumAgeActivatesFault(),
        testTimeoutDetectsClockRegression(),
        testZeroMaximumAgeRuleIsRejected(),
        testMismatchedTimeoutErrorIsRejected(),
        testRateOfChangeRuleIsRejectedBeforeEvaluation(),
        testResultEnumHasFixedWidthStorage()
    };

    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " condition evaluator test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All condition evaluator tests passed\n";
    return EXIT_SUCCESS;
}
