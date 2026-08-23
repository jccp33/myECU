#include "control.hpp"

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

EcuStateInputs inputs(
    const FaultSummary& faults = FaultSummary(),
    bool initializationComplete = true,
    SelfTestResult selfTest = SelfTestResult::PASSED,
    bool shutdownRequested = false,
    bool shutdownPermitted = false,
    DiagnosticStatus diagnostics = DiagnosticStatus::AVAILABLE
) {
    return EcuStateInputs(
        faults, initializationComplete, selfTest, shutdownRequested,
        shutdownPermitted, diagnostics
    );
}

void initializeOperational(Control& control) {
    control.processInputs(inputs());
    control.processInputs(inputs());
}

bool testInitialAndResetState() {
    Control control;
    if (!expectState("Control starts in INIT", control, EcuState::INIT)) return false;
    initializeOperational(control);
    control.reset();
    return expectState("Control reset returns to INIT", control, EcuState::INIT);
}

bool testInitializationSequence() {
    Control control;
    control.processInputs(inputs());
    if (!expectState("Control enters SELF_TEST", control, EcuState::SELF_TEST)) return false;
    control.processInputs(inputs());
    return expectState("Control enters OPERATIONAL", control, EcuState::OPERATIONAL);
}

bool testFaultSummaryDrivesControl() {
    Control control;
    initializeOperational(control);
    control.processInputs(inputs(FaultSummary(false, false, true, 1U)));
    if (!expectState("degraded summary drives DEGRADED", control, EcuState::DEGRADED)) return false;
    control.processInputs(inputs());
    if (!expectState("healthy summary recovers OPERATIONAL", control, EcuState::OPERATIONAL)) return false;
    control.processInputs(inputs(FaultSummary(true, false, false, 1U)));
    return expectState("critical summary drives SAFE_STATE", control, EcuState::SAFE_STATE);
}

bool testDiagnosticFailureDrivesSafeState() {
    Control control;
    initializeOperational(control);
    control.processInputs(inputs(
        FaultSummary(), true, SelfTestResult::PASSED, false, false,
        DiagnosticStatus::CLOCK_ERROR
    ));
    return expectState("diagnostic error drives SAFE_STATE", control, EcuState::SAFE_STATE);
}

bool testShutdownRequestSequence() {
    Control control;
    initializeOperational(control);
    control.processInputs(inputs(FaultSummary(), true, SelfTestResult::PASSED, true, false));
    if (!expectState("shutdown request enters SHUTDOWN_REQ", control, EcuState::SHUTDOWN_REQ)) return false;
    control.processInputs(inputs(FaultSummary(), true, SelfTestResult::PASSED, true, false));
    if (!expectState("SHUTDOWN_REQ waits for permission", control, EcuState::SHUTDOWN_REQ)) return false;
    control.processInputs(inputs(FaultSummary(), true, SelfTestResult::PASSED, true, true));
    return expectState("shutdown permission enters SHUTDOWN", control, EcuState::SHUTDOWN);
}

bool testLatchedCriticalFaultShutsDown() {
    Control control;
    initializeOperational(control);
    const FaultSummary latched(false, true, false, 1U);
    control.processInputs(inputs(latched));
    if (!expectState("latched critical fault first enters SAFE_STATE", control, EcuState::SAFE_STATE)) return false;
    control.processInputs(inputs(latched));
    return expectState("latched critical fault then enters SHUTDOWN", control, EcuState::SHUTDOWN);
}

bool testShutdownIsTerminal() {
    Control control;
    initializeOperational(control);
    control.processInputs(inputs(FaultSummary(), true, SelfTestResult::PASSED, true, false));
    control.processInputs(inputs(FaultSummary(), true, SelfTestResult::PASSED, true, true));
    control.processInputs(EcuStateInputs());
    return expectState("SHUTDOWN remains terminal", control, EcuState::SHUTDOWN);
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testInitialAndResetState(),
        testInitializationSequence(),
        testFaultSummaryDrivesControl(),
        testDiagnosticFailureDrivesSafeState(),
        testShutdownRequestSequence(),
        testLatchedCriticalFaultShutsDown(),
        testShutdownIsTerminal()
    };
    const std::size_t count = sizeof(results) / sizeof(results[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!results[index]) ++failures;
    }
    if (failures != 0) return EXIT_FAILURE;
    std::cout << "All Control tests passed\n";
    return EXIT_SUCCESS;
}
