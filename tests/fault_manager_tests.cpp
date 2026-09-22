#include "fault_manager.hpp"
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
} // namespace

int main() {
    const SignalId degradedId(1U, 1U, 100U, 0U);
    const SignalId criticalId(1U, 1U, 101U, 0U);
    const EvaluationRule rules[] = {
        EvaluationRule(degradedId, 100U, 200U, FaultSeverity::DEGRADED, FaultLatching::RECOVERABLE),
        EvaluationRule(criticalId, 0U, 200U, FaultSeverity::CRITICAL, FaultLatching::LATCHED)
    };
    FaultManager manager(rules, 2U);
    const bool configuration = expectTrue("valid configuration is accepted",
        manager.isConfigValid() && manager.getRuleCount() == 2U);
    const bool unknown = expectTrue("unknown signal is reported",
        manager.processCondition(SignalId(9U, 9U, 9U, 9U), true, 0U)
            == FaultManagerResult::SIGNAL_NOT_CONFIGURED);
    const bool pending = expectTrue("degraded fault starts pending",
        manager.processCondition(degradedId, true, 1000U) == FaultManagerResult::OK
            && manager.getRecord(degradedId)->state == FaultState::PENDING);
    manager.processCondition(degradedId, true, 1100U);
    const FaultSummary degradedSummary = manager.getSummary();
    const bool degraded = expectTrue("confirmed degraded fault appears in summary",
        degradedSummary.hasDegraded && degradedSummary.activeFaultCount == 1U);
    manager.processCondition(criticalId, true, 1100U);
    const FaultSummary criticalSummary = manager.getSummary();
    const bool critical = expectTrue("latched critical fault appears in summary",
        criticalSummary.hasCriticalLatched && criticalSummary.activeFaultCount == 2U);
    const bool clock = expectTrue("clock regression is reported",
        manager.processCondition(degradedId, true, 1099U) == FaultManagerResult::CLOCK_ERROR);
    manager.resetForIgnitionCycle();
    const bool reset = expectTrue("ignition reset clears every record",
        manager.getRecord(degradedId)->state == FaultState::INACTIVE
            && manager.getRecord(criticalId)->state == FaultState::INACTIVE
            && !manager.getSummary().hasActiveFaults());
    const bool invalid = expectTrue("null nonempty configuration is rejected",
        !FaultManager(nullptr, 1U).isConfigValid());
    return configuration && unknown && pending && degraded && critical && clock && reset && invalid
        ? EXIT_SUCCESS : EXIT_FAILURE;
}
