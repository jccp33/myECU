#ifndef ECU_STATE_MACHINE_HPP
#define ECU_STATE_MACHINE_HPP

#include "data_types.hpp"
#include "diagnostic_status.hpp"
#include "fault_types.hpp"
#include <cstdint>

enum class SelfTestResult : std::uint8_t {
    NOT_COMPLETED = 0,
    PASSED,
    FAILED
};

struct EcuStateInputs {
    FaultSummary faults;
    bool initializationComplete;
    SelfTestResult selfTestResult;
    bool shutdownRequested;
    bool shutdownPermitted;
    DiagnosticStatus diagnosticStatus;
    constexpr EcuStateInputs (
        const FaultSummary &faultSummary = FaultSummary(),
        bool initComplete = false,
        SelfTestResult testResult = SelfTestResult::NOT_COMPLETED,
        bool shutdownRequest = false,
        bool shutdownPermission = false,
        DiagnosticStatus diagnostics = DiagnosticStatus::NOT_AVAILABLE
    ) :
        faults(faultSummary),
        initializationComplete(initComplete),
        selfTestResult(testResult),
        shutdownRequested(shutdownRequest),
        shutdownPermitted(shutdownPermission),
        diagnosticStatus(diagnostics) {}
};

EcuState updateEcuState(EcuState currentState, const EcuStateInputs &inputs);

#endif
