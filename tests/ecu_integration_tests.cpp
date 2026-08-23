#include "config.hpp"
#include "control.hpp"
#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include "signal_store.hpp"

#include <cstdlib>
#include <iostream>

namespace {

bool expectTrue(const char* name, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}

float healthyValue(const InitValues& signal) {
    return signal.minValue + ((signal.maxValue - signal.minValue) / 2.0F);
}

void populateHealthySignals(
    const SystemConfig& config,
    SignalStore& store,
    TimestampMs nowMs
) {
    for (std::size_t index = 0U; index < config.sensorCount; ++index) {
        store.upsert(SignalSample(
            config.sensors[index].signalId,
            healthyValue(config.sensors[index]),
            nowMs,
            SignalValidity::VALID
        ));
    }
}

DiagnosticStatus runCycle(
    FaultManager& manager,
    const SignalStore& store,
    Control& control,
    TimestampMs nowMs,
    bool shutdownRequested = false,
    bool shutdownPermitted = false
) {
    const FaultManagerResult result = manager.processCycle(store, nowMs);
    const DiagnosticStatus status = toDiagnosticStatus(result);
    control.processInputs(EcuStateInputs(
        manager.getSummary(),
        true,
        SelfTestResult::PASSED,
        shutdownRequested,
        shutdownPermitted,
        status
    ));
    return status;
}

const InitValues* findSignal(const SystemConfig& config, SensorId sensorId) {
    for (std::size_t index = 0U; index < config.sensorCount; ++index) {
        if (config.sensors[index].sId == sensorId) {
            return &config.sensors[index];
        }
    }
    return nullptr;
}

bool testHealthyConfiguredSystemBecomesOperational() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet rules = getEvaluationRuleSet();
    SignalStore store;
    FaultManager manager(rules.rules, rules.count);
    Control control;
    populateHealthySignals(config, store, 1000U);
    const DiagnosticStatus first = runCycle(manager, store, control, 1000U);
    const DiagnosticStatus second = runCycle(manager, store, control, 1010U);
    return expectTrue(
        "healthy configured system reaches OPERATIONAL",
        first == DiagnosticStatus::AVAILABLE
            && second == DiagnosticStatus::AVAILABLE
            && control.getCurrentState() == EcuState::OPERATIONAL
            && !manager.getSummary().hasActiveFaults()
    );
}

bool testDegradedFaultDebouncesAndRecovers() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet rules = getEvaluationRuleSet();
    const InitValues* speed = findSignal(config, SensorId::SPEED);
    if (speed == nullptr) return false;
    SignalStore store;
    FaultManager manager(rules.rules, rules.count);
    Control control;
    populateHealthySignals(config, store, 1000U);
    runCycle(manager, store, control, 1000U);
    runCycle(manager, store, control, 1010U);

    store.upsert(SignalSample(speed->signalId, 300.0F, 1100U, SignalValidity::VALID));
    runCycle(manager, store, control, 1100U);
    if (!expectTrue(
            "degraded fault remains filtered while pending",
            control.getCurrentState() == EcuState::OPERATIONAL)) return false;
    runCycle(manager, store, control, 1300U);
    if (!expectTrue(
            "confirmed speed fault drives DEGRADED",
            control.getCurrentState() == EcuState::DEGRADED
                && manager.getSummary().hasDegraded)) return false;

    store.upsert(SignalSample(speed->signalId, 100.0F, 1310U, SignalValidity::VALID));
    runCycle(manager, store, control, 1310U);
    runCycle(manager, store, control, 1810U);
    return expectTrue(
        "recovered speed fault returns OPERATIONAL",
        control.getCurrentState() == EcuState::OPERATIONAL
            && !manager.getSummary().hasActiveFaults()
    );
}

bool testCriticalFaultDrivesSafeState() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet rules = getEvaluationRuleSet();
    const InitValues* rpm = findSignal(config, SensorId::RPM);
    if (rpm == nullptr) return false;
    SignalStore store;
    FaultManager manager(rules.rules, rules.count);
    Control control;
    populateHealthySignals(config, store, 1000U);
    runCycle(manager, store, control, 1000U);
    runCycle(manager, store, control, 1010U);
    store.upsert(SignalSample(rpm->signalId, 8000.0F, 1100U, SignalValidity::VALID));
    runCycle(manager, store, control, 1100U);
    runCycle(manager, store, control, 1300U);
    return expectTrue(
        "confirmed RPM fault drives SAFE_STATE",
        manager.getSummary().hasCriticalActive
            && control.getCurrentState() == EcuState::SAFE_STATE
    );
}

bool testLatchedFaultCompletesShutdownPath() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet rules = getEvaluationRuleSet();
    const InitValues* voltage = findSignal(config, SensorId::VOLTAGE);
    if (voltage == nullptr) return false;
    SignalStore store;
    FaultManager manager(rules.rules, rules.count);
    Control control;
    populateHealthySignals(config, store, 1000U);
    runCycle(manager, store, control, 1000U);
    runCycle(manager, store, control, 1010U);
    store.upsert(SignalSample(voltage->signalId, 20.0F, 1100U, SignalValidity::VALID));
    runCycle(manager, store, control, 1100U);
    runCycle(manager, store, control, 1300U);
    if (!expectTrue(
            "latched voltage fault first drives SAFE_STATE",
            manager.getSummary().hasCriticalLatched
                && control.getCurrentState() == EcuState::SAFE_STATE)) return false;
    runCycle(manager, store, control, 1310U);
    return expectTrue(
        "latched voltage fault completes SHUTDOWN path",
        control.getCurrentState() == EcuState::SHUTDOWN
    );
}

bool testMissingCriticalSignalTimesOut() {
    const SystemConfig config = getSystemConfig();
    const EvaluationRuleSet rules = getEvaluationRuleSet();
    const InitValues* rpm = findSignal(config, SensorId::RPM);
    if (rpm == nullptr) return false;
    SignalStore store;
    FaultManager manager(rules.rules, rules.count);
    Control control;
    populateHealthySignals(config, store, 1000U);
    store.clear();
    for (std::size_t index = 0U; index < config.sensorCount; ++index) {
        if (config.sensors[index].signalId != rpm->signalId) {
            store.upsert(SignalSample(
                config.sensors[index].signalId,
                healthyValue(config.sensors[index]),
                1000U,
                SignalValidity::VALID
            ));
        }
    }
    runCycle(manager, store, control, 1000U);
    runCycle(manager, store, control, 1010U);
    runCycle(manager, store, control, 1501U);
    runCycle(manager, store, control, 1701U);
    return expectTrue(
        "missing critical RPM signal times out into SAFE_STATE",
        manager.getSummary().hasCriticalActive
            && control.getCurrentState() == EcuState::SAFE_STATE
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testHealthyConfiguredSystemBecomesOperational(),
        testDegradedFaultDebouncesAndRecovers(),
        testCriticalFaultDrivesSafeState(),
        testLatchedFaultCompletesShutdownPath(),
        testMissingCriticalSignalTimesOut()
    };
    const std::size_t count = sizeof(results) / sizeof(results[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!results[index]) ++failures;
    }
    if (failures != 0) return EXIT_FAILURE;
    std::cout << "All ECU integration tests passed\n";
    return EXIT_SUCCESS;
}
