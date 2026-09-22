#include "control.hpp"
#include <array>
#include <cstdlib>
#include <iostream>

namespace {
bool expectState(const char* name, const Control& control, EcuState expected) {
    if (control.getCurrentState() != expected) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}

Message message(const SignalId& id, SignalStatus status, FaultSeverity severityValue = FaultSeverity::WARNING) {
    (void)severityValue;
    Message result(id, 0.0F, 0.0F, 1.0F, 500U, false, false, 1.0F, 0U);
    result.setSignalStatus(status);
    return result;
}
} // namespace

int main() {
    const SignalId id(1U, 1U, 100U, 0U);
    const EvaluationRule healthyRule(id, 0U, 0U, FaultSeverity::WARNING, FaultLatching::RECOVERABLE);
    FaultManager healthyManager(&healthyRule, 1U);
    std::array<Message, MAX_SENSOR_COUNT> messages;
    messages[0] = message(id, SignalStatus::VALID);
    Control control;
    bool passed = expectState("Control starts in INIT", control, EcuState::INIT);
    passed = control.processMessages(messages, 1U, healthyManager, 0U) == FaultManagerResult::OK
        && expectState("first cycle advances to SELF_TEST", control, EcuState::SELF_TEST) && passed;
    passed = control.processMessages(messages, 1U, healthyManager, 1U) == FaultManagerResult::OK
        && expectState("second healthy cycle becomes OPERATIONAL", control, EcuState::OPERATIONAL) && passed;

    const SignalId degradedId(1U, 1U, 101U, 0U);
    const EvaluationRule degradedRule(degradedId, 0U, 0U, FaultSeverity::DEGRADED, FaultLatching::RECOVERABLE);
    FaultManager degradedManager(&degradedRule, 1U);
    messages[0] = message(degradedId, SignalStatus::OUT_OF_RANGE);
    Control degradedControl;
    degradedControl.processMessages(messages, 1U, degradedManager, 0U);
    degradedControl.processMessages(messages, 1U, degradedManager, 1U);
    passed = expectState("degraded fault drives DEGRADED", degradedControl, EcuState::DEGRADED) && passed;

    const SignalId criticalId(1U, 1U, 102U, 0U);
    const EvaluationRule criticalRule(criticalId, 0U, 0U, FaultSeverity::CRITICAL, FaultLatching::RECOVERABLE);
    FaultManager criticalManager(&criticalRule, 1U);
    messages[0] = message(criticalId, SignalStatus::TIMEOUT);
    Control criticalControl;
    criticalControl.processMessages(messages, 1U, criticalManager, 0U);
    criticalControl.processMessages(messages, 1U, criticalManager, 1U);
    passed = expectState("critical fault drives SAFE_STATE", criticalControl, EcuState::SAFE_STATE) && passed;

    messages[0] = message(id, SignalStatus::UNDEFINED);
    Control invalidControl;
    invalidControl.processMessages(messages, 1U, healthyManager, 2U);
    invalidControl.processMessages(messages, 1U, healthyManager, 3U);
    passed = expectState("undefined signal fails safe", invalidControl, EcuState::SAFE_STATE) && passed;

    control.reset();
    passed = expectState("reset returns to INIT", control, EcuState::INIT) && passed;
    passed = control.processMessages(messages, messages.size() + 1U, healthyManager, 0U)
        == FaultManagerResult::INVALID_CONFIGURATION && passed;
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
