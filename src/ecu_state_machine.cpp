#include "../include/ecu_state_machine.hpp"

EcuState updateEcuState(EcuState currentState, const EcuStateInputs &inputs){
    const bool criticalFault = inputs.faults.hasCriticalActive || inputs.faults.hasCriticalLatched;
    const bool diagnosticFailure = !isDiagnosticAvailable(inputs.diagnosticStatus);
    switch(currentState){
        case EcuState::INIT:
            if(inputs.shutdownRequested) return EcuState::SHUTDOWN_REQ;
            if(inputs.initializationComplete) return EcuState::SELF_TEST;
            return EcuState::INIT;
        case EcuState::SELF_TEST:
            if(inputs.selfTestResult == SelfTestResult::FAILED) return EcuState::SAFE_STATE;
            if(inputs.selfTestResult != SelfTestResult::PASSED) return EcuState::SELF_TEST;
            if(diagnosticFailure) return EcuState::SAFE_STATE;
            if(criticalFault) return EcuState::SAFE_STATE;
            if(inputs.shutdownRequested) return EcuState::SHUTDOWN_REQ;
            if(inputs.faults.hasDegraded) return EcuState::DEGRADED;
            return EcuState::OPERATIONAL;
        case EcuState::OPERATIONAL:
            if(diagnosticFailure) return EcuState::SAFE_STATE;
            if(criticalFault) return EcuState::SAFE_STATE;
            if(inputs.shutdownRequested) return EcuState::SHUTDOWN_REQ;
            if(inputs.faults.hasDegraded) return  EcuState::DEGRADED;
            return EcuState::OPERATIONAL;
        case EcuState::DEGRADED:
            if(diagnosticFailure) return EcuState::SAFE_STATE;
            if(criticalFault) return EcuState::SAFE_STATE;
            if(inputs.shutdownRequested) return EcuState::SHUTDOWN_REQ;
            if(inputs.faults.hasDegraded) return EcuState::DEGRADED;
            return EcuState::OPERATIONAL;
        case EcuState::SAFE_STATE:
            if(inputs.faults.hasCriticalLatched) return EcuState::SHUTDOWN;
            if(inputs.shutdownRequested) return EcuState::SHUTDOWN_REQ;
            if(diagnosticFailure) return EcuState::SAFE_STATE;
            if(criticalFault) return EcuState::SAFE_STATE;
            if(inputs.faults.hasDegraded) return EcuState::DEGRADED;
            return EcuState::OPERATIONAL;
        case EcuState::SHUTDOWN_REQ:
            if(inputs.shutdownPermitted) return EcuState::SHUTDOWN;
            return EcuState::SHUTDOWN_REQ;
        case EcuState::SHUTDOWN:
            return EcuState::SHUTDOWN;
    }
    return EcuState::SAFE_STATE;
}
