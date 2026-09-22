#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include <cstdlib>
#include <iostream>
#include <type_traits>

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
    const bool passed =
        expectTrue("configuration results map to diagnostic status",
            toDiagnosticStatus(FaultConfigurationError::NONE) == DiagnosticStatus::AVAILABLE
            && toDiagnosticStatus(FaultConfigurationError::INVALID_SYSTEM_CONFIGURATION) == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultConfigurationError::DUPLICATE_SIGNAL) == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultConfigurationError::INVALID_RULE) == DiagnosticStatus::CONFIGURATION_ERROR)
        && expectTrue("manager results map to diagnostic status",
            toDiagnosticStatus(FaultManagerResult::OK) == DiagnosticStatus::AVAILABLE
            && toDiagnosticStatus(FaultManagerResult::INVALID_CONFIGURATION) == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultManagerResult::SIGNAL_NOT_CONFIGURED) == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultManagerResult::CLOCK_ERROR) == DiagnosticStatus::CLOCK_ERROR)
        && expectTrue("only AVAILABLE is available",
            isDiagnosticAvailable(DiagnosticStatus::AVAILABLE)
            && !isDiagnosticAvailable(DiagnosticStatus::NOT_AVAILABLE)
            && !isDiagnosticAvailable(DiagnosticStatus::CONFIGURATION_ERROR)
            && !isDiagnosticAvailable(DiagnosticStatus::CLOCK_ERROR)
            && !isDiagnosticAvailable(DiagnosticStatus::EVALUATION_ERROR))
        && expectTrue("DiagnosticStatus uses uint8_t storage",
            std::is_same<std::underlying_type<DiagnosticStatus>::type, std::uint8_t>::value);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
