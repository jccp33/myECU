#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"

DiagnosticStatus toDiagnosticStatus(FaultConfigurationError error) {
    if (error == FaultConfigurationError::NONE) {
        return DiagnosticStatus::AVAILABLE;
    }
    return DiagnosticStatus::CONFIGURATION_ERROR;
}

DiagnosticStatus toDiagnosticStatus(FaultManagerResult result) {
    switch (result) {
        case FaultManagerResult::OK:
            return DiagnosticStatus::AVAILABLE;
        case FaultManagerResult::INVALID_CONFIGURATION:
        case FaultManagerResult::SIGNAL_NOT_CONFIGURED:
            return DiagnosticStatus::CONFIGURATION_ERROR;
        case FaultManagerResult::CLOCK_ERROR:
            return DiagnosticStatus::CLOCK_ERROR;
    }
    return DiagnosticStatus::EVALUATION_ERROR;
}
