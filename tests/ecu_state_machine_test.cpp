#include "ecu_state_machine.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

struct TransitionCase {
    const char* name;
    EcuState current;
    EcuStateInputs inputs;
    EcuState expected;
};

bool expectTransition(const TransitionCase& test) {
    const EcuState actual = updateEcuState(test.current, test.inputs);
    if (actual != test.expected) {
        std::cerr << "FAILED: " << test.name << '\n';
        return false;
    }

    std::cout << "PASSED: " << test.name << '\n';
    return true;
}

FaultSummary degradedFault() {
    return FaultSummary(false, false, true, 1U);
}

FaultSummary criticalActiveFault() {
    return FaultSummary(true, false, false, 1U);
}

FaultSummary criticalLatchedFault() {
    return FaultSummary(false, true, false, 1U);
}

EcuStateInputs availableInputs(
    const FaultSummary& faults = FaultSummary(),
    bool initializationComplete = false,
    SelfTestResult selfTestResult = SelfTestResult::NOT_COMPLETED,
    bool shutdownRequested = false,
    bool shutdownPermitted = false
) {
    return EcuStateInputs(
        faults,
        initializationComplete,
        selfTestResult,
        shutdownRequested,
        shutdownPermitted,
        DiagnosticStatus::AVAILABLE
    );
}

bool testTransitionTable() {
    const TransitionCase cases[] = {
        {
            "INIT remains INIT while initialization is incomplete",
            EcuState::INIT,
            availableInputs(),
            EcuState::INIT
        },
        {
            "INIT enters SELF_TEST when initialization completes",
            EcuState::INIT,
            availableInputs(FaultSummary(), true),
            EcuState::SELF_TEST
        },
        {
            "SELF_TEST waits while result is not completed",
            EcuState::SELF_TEST,
            availableInputs(),
            EcuState::SELF_TEST
        },
        {
            "failed SELF_TEST enters SAFE_STATE",
            EcuState::SELF_TEST,
            availableInputs(FaultSummary(), true, SelfTestResult::FAILED),
            EcuState::SAFE_STATE
        },
        {
            "passed healthy SELF_TEST enters OPERATIONAL",
            EcuState::SELF_TEST,
            availableInputs(FaultSummary(), true, SelfTestResult::PASSED),
            EcuState::OPERATIONAL
        },
        {
            "passed SELF_TEST with degraded fault enters DEGRADED",
            EcuState::SELF_TEST,
            availableInputs(degradedFault(), true, SelfTestResult::PASSED),
            EcuState::DEGRADED
        },
        {
            "passed SELF_TEST with active critical fault enters SAFE_STATE",
            EcuState::SELF_TEST,
            availableInputs(criticalActiveFault(), true, SelfTestResult::PASSED),
            EcuState::SAFE_STATE
        },
        {
            "passed SELF_TEST with latched critical fault enters SAFE_STATE",
            EcuState::SELF_TEST,
            availableInputs(criticalLatchedFault(), true, SelfTestResult::PASSED),
            EcuState::SAFE_STATE
        },
        {
            "healthy OPERATIONAL remains OPERATIONAL",
            EcuState::OPERATIONAL,
            availableInputs(),
            EcuState::OPERATIONAL
        },
        {
            "degraded fault moves OPERATIONAL to DEGRADED",
            EcuState::OPERATIONAL,
            availableInputs(degradedFault()),
            EcuState::DEGRADED
        },
        {
            "critical fault moves OPERATIONAL to SAFE_STATE",
            EcuState::OPERATIONAL,
            availableInputs(criticalActiveFault()),
            EcuState::SAFE_STATE
        },
        {
            "persistent degraded fault keeps DEGRADED",
            EcuState::DEGRADED,
            availableInputs(degradedFault()),
            EcuState::DEGRADED
        },
        {
            "recovery moves DEGRADED to OPERATIONAL",
            EcuState::DEGRADED,
            availableInputs(),
            EcuState::OPERATIONAL
        },
        {
            "critical fault has priority over shutdown in DEGRADED",
            EcuState::DEGRADED,
            availableInputs(criticalActiveFault(), false,
                           SelfTestResult::NOT_COMPLETED, true),
            EcuState::SAFE_STATE
        },
        {
            "shutdown request moves DEGRADED to SHUTDOWN_REQ",
            EcuState::DEGRADED,
            availableInputs(FaultSummary(), false,
                           SelfTestResult::NOT_COMPLETED, true),
            EcuState::SHUTDOWN_REQ
        },
        {
            "critical fault keeps SAFE_STATE",
            EcuState::SAFE_STATE,
            availableInputs(criticalActiveFault()),
            EcuState::SAFE_STATE
        },
        {
            "latched critical fault moves SAFE_STATE to SHUTDOWN",
            EcuState::SAFE_STATE,
            availableInputs(criticalLatchedFault()),
            EcuState::SHUTDOWN
        },
        {
            "remaining degraded fault moves SAFE_STATE to DEGRADED",
            EcuState::SAFE_STATE,
            availableInputs(degradedFault()),
            EcuState::DEGRADED
        },
        {
            "complete recovery moves SAFE_STATE to OPERATIONAL",
            EcuState::SAFE_STATE,
            availableInputs(),
            EcuState::OPERATIONAL
        },
        {
            "shutdown request moves SAFE_STATE to SHUTDOWN_REQ",
            EcuState::SAFE_STATE,
            availableInputs(criticalActiveFault(), false,
                           SelfTestResult::NOT_COMPLETED, true),
            EcuState::SHUTDOWN_REQ
        },
        {
            "shutdown request moves OPERATIONAL to SHUTDOWN_REQ",
            EcuState::OPERATIONAL,
            availableInputs(FaultSummary(), false,
                           SelfTestResult::NOT_COMPLETED, true),
            EcuState::SHUTDOWN_REQ
        },
        {
            "SHUTDOWN_REQ waits for shutdown permission",
            EcuState::SHUTDOWN_REQ,
            availableInputs(),
            EcuState::SHUTDOWN_REQ
        },
        {
            "SHUTDOWN_REQ enters SHUTDOWN when permitted",
            EcuState::SHUTDOWN_REQ,
            availableInputs(FaultSummary(), false,
                           SelfTestResult::NOT_COMPLETED, true, true),
            EcuState::SHUTDOWN
        },
        {
            "SHUTDOWN is terminal",
            EcuState::SHUTDOWN,
            availableInputs(criticalActiveFault(), true,
                           SelfTestResult::FAILED, true),
            EcuState::SHUTDOWN
        },
        {
            "unknown state fails safe",
            static_cast<EcuState>(255),
            availableInputs(),
            EcuState::SAFE_STATE
        }
    };

    bool passed = true;
    const std::size_t count = sizeof(cases) / sizeof(cases[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!expectTransition(cases[index])) {
            passed = false;
        }
    }
    return passed;
}

bool testWarningOnlyDoesNotDegradeOperation() {
    const FaultSummary warningOnly(false, false, false, 1U);
    return expectTransition({
        "warning-only fault does not degrade operation",
        EcuState::OPERATIONAL,
        availableInputs(warningOnly),
        EcuState::OPERATIONAL
    });
}

bool testUnavailableDiagnosticsFailSafe() {
    const DiagnosticStatus failures[] = {
        DiagnosticStatus::NOT_AVAILABLE,
        DiagnosticStatus::CONFIGURATION_ERROR,
        DiagnosticStatus::CLOCK_ERROR,
        DiagnosticStatus::EVALUATION_ERROR,
        DiagnosticStatus::INVALID_SIGNAL_SAMPLE
    };

    bool passed = true;
    const std::size_t count = sizeof(failures) / sizeof(failures[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        const EcuStateInputs inputs(
            FaultSummary(),
            true,
            SelfTestResult::PASSED,
            false,
            false,
            failures[index]
        );
        if (updateEcuState(EcuState::OPERATIONAL, inputs)
                != EcuState::SAFE_STATE) {
            passed = false;
        }
        if (updateEcuState(EcuState::DEGRADED, inputs)
                != EcuState::SAFE_STATE) {
            passed = false;
        }
        if (updateEcuState(EcuState::SAFE_STATE, inputs)
                != EcuState::SAFE_STATE) {
            passed = false;
        }
        if (updateEcuState(EcuState::SELF_TEST, inputs)
                != EcuState::SAFE_STATE) {
            passed = false;
        }
    }

    return expectTransition({
        "unavailable diagnostics fail safe",
        EcuState::OPERATIONAL,
        EcuStateInputs(),
        passed ? EcuState::SAFE_STATE : EcuState::OPERATIONAL
    });
}

bool testPendingSelfTestWaitsForDiagnostics() {
    return expectTransition({
        "pending SELF_TEST waits while diagnostics are unavailable",
        EcuState::SELF_TEST,
        EcuStateInputs(),
        EcuState::SELF_TEST
    });
}

bool testSelfTestResultHasFixedWidthStorage() {
    const bool fixedWidth = std::is_same<
        std::underlying_type<SelfTestResult>::type,
        std::uint8_t
    >::value;

    if (!fixedWidth) {
        std::cerr << "FAILED: SelfTestResult uses uint8_t storage\n";
        return false;
    }
    std::cout << "PASSED: SelfTestResult uses uint8_t storage\n";
    return true;
}

}  // namespace

int main() {
    const bool transitionTablePassed = testTransitionTable();
    const bool warningPassed = testWarningOnlyDoesNotDegradeOperation();
    const bool diagnosticPassed = testUnavailableDiagnosticsFailSafe();
    const bool pendingDiagnosticsPassed = testPendingSelfTestWaitsForDiagnostics();
    const bool storagePassed = testSelfTestResultHasFixedWidthStorage();

    if (!transitionTablePassed
            || !warningPassed
            || !diagnosticPassed
            || !pendingDiagnosticsPassed
            || !storagePassed) {
        return EXIT_FAILURE;
    }

    std::cout << "All ECU state machine tests passed\n";
    return EXIT_SUCCESS;
}
