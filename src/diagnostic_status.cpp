#include "../include/diagnostic_status.hpp"
#include "../include/fault_configuration.hpp"
#include "../include/fault_manager.hpp"

DiagnosticStatus toDiagnosticStatus(FaultConfigurationError error) {
    if (error == FaultConfigurationError::NONE) {
        return DiagnosticStatus::AVAILABLE;
    }
    return DiagnosticStatus::CONFIGURATION_ERROR;
}

DiagnosticStatus toDiagnosticStatus(FaultManagerResult result) {
    switch (result) {
        case FaultManagerResult::OK:
        case FaultManagerResult::NO_MATCHING_RULE:
            return DiagnosticStatus::AVAILABLE;
        case FaultManagerResult::INVALID_CONFIGURATION:
            return DiagnosticStatus::CONFIGURATION_ERROR;
        case FaultManagerResult::CLOCK_ERROR:
            return DiagnosticStatus::CLOCK_ERROR;
        case FaultManagerResult::INVALID_RULE_INDEX:
        case FaultManagerResult::RULE_EVALUATION_ERROR:
            return DiagnosticStatus::EVALUATION_ERROR;
        case FaultManagerResult::INVALID_SIGNAL_SAMPLE:
            return DiagnosticStatus::INVALID_SIGNAL_SAMPLE;
    }
    return DiagnosticStatus::EVALUATION_ERROR;
}
