#include "fault_state_machine.hpp"
#include <cstdlib>
#include <iostream>

namespace {
bool expect(const char* name, const FaultRecord& actual, FaultState state, TimestampMs entry) {
    if (actual.state != state || actual.stateEntryTimeMs != entry) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}
EvaluationRule rule(TimestampMs confirmation, TimestampMs recovery, FaultLatching latching) {
    return EvaluationRule(SignalId(1U, 1U, 100U, 0U), confirmation, recovery,
        FaultSeverity::CRITICAL, latching);
}
} // namespace

int main() {
    const EvaluationRule recoverable = rule(200U, 500U, FaultLatching::RECOVERABLE);
    const FaultRecord pending = updateFaultRecord(recoverable, FaultRecord(), true, 1000U);
    const FaultRecord stillPending = updateFaultRecord(recoverable, pending, true, 1199U);
    const FaultRecord confirmed = updateFaultRecord(recoverable, pending, true, 1200U);
    const FaultRecord recovering = updateFaultRecord(recoverable, confirmed, false, 1300U);
    const FaultRecord stillRecovering = updateFaultRecord(recoverable, recovering, false, 1799U);
    const FaultRecord inactive = updateFaultRecord(recoverable, recovering, false, 1800U);
    const EvaluationRule latchedRule = rule(0U, 500U, FaultLatching::LATCHED);
    const FaultRecord latched = updateFaultRecord(latchedRule, FaultRecord(), true, 2000U);
    const bool passed =
        expect("inactive condition remains inactive", updateFaultRecord(recoverable, FaultRecord(), false, 1000U), FaultState::INACTIVE, 0U)
        && expect("fault starts pending", pending, FaultState::PENDING, 1000U)
        && expect("confirmation waits for threshold", stillPending, FaultState::PENDING, 1000U)
        && expect("confirmation occurs at threshold", confirmed, FaultState::CONFIRMED, 1200U)
        && expect("healthy confirmed fault starts recovery", recovering, FaultState::RECOVERING, 1300U)
        && expect("recovery waits for threshold", stillRecovering, FaultState::RECOVERING, 1300U)
        && expect("recovery completes at threshold", inactive, FaultState::INACTIVE, 1800U)
        && expect("reappearing fault cancels recovery", updateFaultRecord(recoverable, recovering, true, 1400U), FaultState::CONFIRMED, 1400U)
        && expect("zero confirmation latches immediately", latched, FaultState::LATCHED, 2000U)
        && expect("latched fault stays latched", updateFaultRecord(latchedRule, latched, false, 3000U), FaultState::LATCHED, 2000U);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
