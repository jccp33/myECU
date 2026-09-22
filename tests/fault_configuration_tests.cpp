#include "config.hpp"
#include "fault_configuration.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

namespace {
bool expect(const char* name, FaultConfigurationError actual, FaultConfigurationError expected) {
    if (actual != expected) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}
} // namespace

int main() {
    const SystemConfig config = getSystemConfig();
    std::array<EvaluationRule, MAX_SENSOR_COUNT> storage;
    EvaluationRuleSet set;
    const bool built = expect("rules are built from SystemConfig",
        buildEvaluationRuleSet(config, storage, set), FaultConfigurationError::NONE);
    const bool shape = set.rules == storage.data() && set.count == config.sensorCount;
    const bool validated = expect("derived rules validate",
        validateEvaluationRuleSet(set, config), FaultConfigurationError::NONE);

    SystemConfig duplicate = config;
    duplicate.sensors[1].signalId = duplicate.sensors[0].signalId;
    const bool duplicateRejected = expect("duplicate signals are rejected",
        buildEvaluationRuleSet(duplicate, storage, set), FaultConfigurationError::DUPLICATE_SIGNAL);

    SystemConfig oversized = config;
    oversized.sensorCount = oversized.sensors.size() + 1U;
    const bool oversizedRejected = expect("oversized configuration is rejected",
        buildEvaluationRuleSet(oversized, storage, set), FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION);

    const EvaluationRule divergent(
        config.sensors[0].signalId,
        config.sensors[0].confirmationTimeMs + 1U,
        config.sensors[0].recoveryTimeMs,
        config.sensors[0].severity,
        config.sensors[0].latching
    );
    const EvaluationRuleSet divergentSet(&divergent, 1U);
    SystemConfig oneSignal = config;
    oneSignal.sensorCount = 1U;
    const bool divergenceRejected = expect("divergent derived policy is rejected",
        validateEvaluationRuleSet(divergentSet, oneSignal), FaultConfigurationError::INVALID_RULE);

    return built && shape && validated && duplicateRejected && oversizedRejected && divergenceRejected
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
