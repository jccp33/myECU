#include "evaluation_rule.hpp"

#include <cstdlib>
#include <iostream>

namespace {

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool expectValidation(
    const char* testName,
    const EvaluationRule& rule,
    RuleValidationError expected
) {
    const RuleValidationError actual = rule.validate();

    if (actual != expected) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testRangeRulePreservesConfiguration() {
    const SignalId rpmSignal(1U, 1U, 100U, 0U);
    const EvaluationRule rpmRule(
        rpmSignal,
        EvaluationType::RANGE,
        0.0F,
        7000.0F,
        200U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::CRITICAL,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectTrue(
        "range rule preserves its complete configuration",
        rpmRule.getSignal() == rpmSignal
            && rpmRule.getEvaluationType() == EvaluationType::RANGE
            && rpmRule.getLowerThreshold() == 0.0F
            && rpmRule.getUpperThreshold() == 7000.0F
            && rpmRule.getConfirmationTimeMs() == 200U
            && rpmRule.getRecoveryTimeMs() == 500U
            && rpmRule.getMaximumAgeMs() == 0U
            && rpmRule.getError() == SignalError::OUT_OF_RANGE
            && rpmRule.getSeverity() == FaultSeverity::CRITICAL
            && rpmRule.getFaultType() == FaultType::SENSOR
            && rpmRule.getLatching() == FaultLatching::RECOVERABLE
    );
}

bool testTimeoutRuleIsConstructible() {
    const SignalId speedSignal(1U, 2U, 101U, 0U);
    const EvaluationRule timeoutRule(
        speedSignal,
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        500U,
        SignalError::TIMEOUT,
        FaultSeverity::DEGRADED,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );

    return expectTrue(
        "timeout rule is usable through its public interface",
        timeoutRule.getSignal() == speedSignal
            && timeoutRule.getEvaluationType() == EvaluationType::TIMEOUT
            && timeoutRule.getMaximumAgeMs() == 500U
            && timeoutRule.getError() == SignalError::TIMEOUT
            && timeoutRule.getSeverity() == FaultSeverity::DEGRADED
    );
}

bool testValidRangeRule() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        0.0F,
        7000.0F,
        200U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::CRITICAL,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "valid RANGE rule is accepted",
        rule,
        RuleValidationError::NONE
    );
}

bool testRangeRuleRejectsInvertedThresholds() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        7000.0F,
        0.0F,
        200U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::CRITICAL,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "RANGE rule rejects inverted thresholds",
        rule,
        RuleValidationError::INVALID_THRESHOLDS
    );
}

bool testRangeRuleRejectsMismatchedError() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        0.0F,
        7000.0F,
        200U,
        500U,
        0U,
        SignalError::TIMEOUT,
        FaultSeverity::CRITICAL,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "RANGE rule rejects a non-range error",
        rule,
        RuleValidationError::ERROR_TYPE_MISMATCH
    );
}

bool testRangeRuleRejectsNoneSeverity() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        0.0F,
        7000.0F,
        200U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::NONE,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "RANGE rule rejects NONE severity",
        rule,
        RuleValidationError::INVALID_SEVERITY
    );
}

bool testValidTimeoutRule() {
    const EvaluationRule rule(
        SignalId(1U, 2U, 101U, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        500U,
        SignalError::TIMEOUT,
        FaultSeverity::DEGRADED,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "valid TIMEOUT rule is accepted",
        rule,
        RuleValidationError::NONE
    );
}

bool testTimeoutRuleRejectsZeroMaximumAge() {
    const EvaluationRule rule(
        SignalId(1U, 2U, 101U, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        0U,
        SignalError::TIMEOUT,
        FaultSeverity::DEGRADED,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "TIMEOUT rule rejects zero maximum age",
        rule,
        RuleValidationError::INVALID_MAXIMUM_AGE
    );
}

bool testTimeoutRuleRejectsMismatchedError() {
    const EvaluationRule rule(
        SignalId(1U, 2U, 101U, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        500U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::DEGRADED,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "TIMEOUT rule rejects a non-timeout error",
        rule,
        RuleValidationError::ERROR_TYPE_MISMATCH
    );
}

bool testTimeoutRuleRejectsNoneSeverity() {
    const EvaluationRule rule(
        SignalId(1U, 2U, 101U, 0U),
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        100U,
        1000U,
        500U,
        SignalError::TIMEOUT,
        FaultSeverity::NONE,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "TIMEOUT rule rejects NONE severity",
        rule,
        RuleValidationError::INVALID_SEVERITY
    );
}

bool testRateOfChangeIsNotSupportedYet() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RATE_OF_CHANGE,
        0.0F,
        1000.0F,
        100U,
        500U,
        0U,
        SignalError::RATE_OF_CHANGE,
        FaultSeverity::WARNING,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "RATE_OF_CHANGE remains unsupported",
        rule,
        RuleValidationError::INVALID_EVALUATION_TYPE
    );
}

bool testUnknownEvaluationTypeIsRejected() {
    const EvaluationRule rule(
        SignalId(1U, 1U, 100U, 0U),
        static_cast<EvaluationType>(255U),
        0.0F,
        1000.0F,
        100U,
        500U,
        0U,
        SignalError::NONE,
        FaultSeverity::WARNING,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );

    return expectValidation(
        "unknown evaluation type is rejected",
        rule,
        RuleValidationError::INVALID_EVALUATION_TYPE
    );
}

}  // namespace

int main() {
    const bool results[] = {
        testRangeRulePreservesConfiguration(),
        testTimeoutRuleIsConstructible(),
        testValidRangeRule(),
        testRangeRuleRejectsInvertedThresholds(),
        testRangeRuleRejectsMismatchedError(),
        testRangeRuleRejectsNoneSeverity(),
        testValidTimeoutRule(),
        testTimeoutRuleRejectsZeroMaximumAge(),
        testTimeoutRuleRejectsMismatchedError(),
        testTimeoutRuleRejectsNoneSeverity(),
        testRateOfChangeIsNotSupportedYet(),
        testUnknownEvaluationTypeIsRejected()
    };
    int failures = 0;

    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " evaluation rule test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All evaluation rule tests passed\n";
    return EXIT_SUCCESS;
}
