#include "config.hpp"
#include "fault_configuration.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

EvaluationRule createValidRangeRule(
    const SignalId& id,
    float lower = 0.0F,
    float upper = 100.0F
) {
    return EvaluationRule(
        id,
        EvaluationType::RANGE,
        lower,
        upper,
        100U,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::DEGRADED,
        FaultType::SENSOR,
        FaultLatching::RECOVERABLE
    );
}

bool testEmptyRuleSetIsValid() {
    const SystemConfig config = getSystemConfig();
    return expectTrue(
        "empty evaluation rule set is valid",
        validateEvaluationRuleSet(EvaluationRuleSet(), config)
            == FaultConfigurationError::NONE
    );
}

bool testNullRulesWithCountIsRejected() {
    const SystemConfig config = getSystemConfig();
    return expectTrue(
        "null evaluation rules with nonzero count are rejected",
        validateEvaluationRuleSet(EvaluationRuleSet(nullptr, 1U), config)
            == FaultConfigurationError::NULL_RULES
    );
}

bool testRuleCountAboveCapacityIsRejectedBeforeDereference() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRule rule = createValidRangeRule(
        config.sensors[0U].signalId
    );
    return expectTrue(
        "evaluation rule count above capacity is rejected safely",
        validateEvaluationRuleSet(
            EvaluationRuleSet(&rule, MAX_EVALUATION_RULE_COUNT + 1U),
            config
        ) == FaultConfigurationError::TOO_MANY_RULES
    );
}

bool testInvalidSystemSensorCountIsRejected() {
    SystemConfig config = getSystemConfig();
    config.sensorCount = config.sensors.size() + 1U;
    return expectTrue(
        "invalid system sensor count is rejected",
        validateEvaluationRuleSet(EvaluationRuleSet(), config)
            == FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION
    );
}

bool testDuplicateConfiguredSignalIsRejected() {
    SystemConfig config = getSystemConfig();
    config.sensors[1U].signalId = config.sensors[0U].signalId;
    return expectTrue(
        "duplicate configured SignalId is rejected",
        validateEvaluationRuleSet(EvaluationRuleSet(), config)
            == FaultConfigurationError::DUPLICATE_SIGNAL
    );
}

bool testInvalidIndividualRuleIsRejected() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRule invalidRule = createValidRangeRule(
        config.sensors[0U].signalId,
        100.0F,
        0.0F
    );
    return expectTrue(
        "invalid individual evaluation rule is rejected",
        validateEvaluationRuleSet(
            EvaluationRuleSet(&invalidRule, 1U), config
        ) == FaultConfigurationError::INVALID_RULE
    );
}

bool testRuleForUnknownSignalIsRejected() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRule rule = createValidRangeRule(
        SignalId(9U, 9U, 999U, 9U)
    );
    return expectTrue(
        "rule for unknown signal is rejected",
        validateEvaluationRuleSet(EvaluationRuleSet(&rule, 1U), config)
            == FaultConfigurationError::UNKNOWN_SIGNAL
    );
}

bool testMaximumRulesPerSignalIsAccepted() {
    const SystemConfig config = getSystemConfig();
    const SignalId id = config.sensors[0U].signalId;
    const EvaluationRule rules[] = {
        createValidRangeRule(id, 0.0F, 100.0F),
        createValidRangeRule(id, -1.0F, 101.0F),
        createValidRangeRule(id, -2.0F, 102.0F)
    };
    return expectTrue(
        "maximum configured rules per signal is accepted",
        validateEvaluationRuleSet(EvaluationRuleSet(rules, 3U), config)
            == FaultConfigurationError::NONE
    );
}

bool testTooManyRulesForOneSignalIsRejected() {
    const SystemConfig config = getSystemConfig();
    const SignalId id = config.sensors[0U].signalId;
    const EvaluationRule rules[] = {
        createValidRangeRule(id, 0.0F, 100.0F),
        createValidRangeRule(id, -1.0F, 101.0F),
        createValidRangeRule(id, -2.0F, 102.0F),
        createValidRangeRule(id, -3.0F, 103.0F)
    };
    return expectTrue(
        "more than maximum rules for one signal is rejected",
        validateEvaluationRuleSet(EvaluationRuleSet(rules, 4U), config)
            == FaultConfigurationError::TOO_MANY_RULES_FOR_SIGNAL
    );
}

bool testStaticRuleSetIsValidAndStable() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet first = getEvaluationRuleSet();
    const EvaluationRuleSet second = getEvaluationRuleSet();
    bool everyRuleValid = true;
    for (std::size_t index = 0U; index < first.count; ++index) {
        if (first.rules[index].validate() != RuleValidationError::NONE) {
            everyRuleValid = false;
        }
    }

    return expectTrue(
        "static evaluation rule set is valid and has stable lifetime",
        first.rules != nullptr
            && first.count == 20U
            && first.rules == second.rules
            && first.count == second.count
            && everyRuleValid
            && validateEvaluationRuleSet(first, config)
                == FaultConfigurationError::NONE
    );
}

bool testFaultConfigurationErrorUsesFixedWidthStorage() {
    return expectTrue(
        "FaultConfigurationError uses uint8_t storage",
        std::is_same<
            std::underlying_type<FaultConfigurationError>::type,
            std::uint8_t
        >::value
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testEmptyRuleSetIsValid(),
        testNullRulesWithCountIsRejected(),
        testRuleCountAboveCapacityIsRejectedBeforeDereference(),
        testInvalidSystemSensorCountIsRejected(),
        testDuplicateConfiguredSignalIsRejected(),
        testInvalidIndividualRuleIsRejected(),
        testRuleForUnknownSignalIsRejected(),
        testMaximumRulesPerSignalIsAccepted(),
        testTooManyRulesForOneSignalIsRejected(),
        testStaticRuleSetIsValidAndStable(),
        testFaultConfigurationErrorUsesFixedWidthStorage()
    };

    const std::size_t count = sizeof(results) / sizeof(results[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!results[index]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " fault configuration test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All fault configuration tests passed\n";
    return EXIT_SUCCESS;
}
