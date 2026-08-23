#ifndef DIAGNOSTIC_STATUS_HPP
#define DIAGNOSTIC_STATUS_HPP

#include <cstdint>

enum class FaultConfigurationError : std::uint8_t;
enum class FaultManagerResult : std::uint8_t;

enum class DiagnosticStatus : std::uint8_t {
    NOT_AVAILABLE = 0,
    AVAILABLE,
    CONFIGURATION_ERROR,
    CLOCK_ERROR,
    EVALUATION_ERROR,
    INVALID_SIGNAL_SAMPLE
};

constexpr bool isDiagnosticAvailable(DiagnosticStatus status) {
    return status == DiagnosticStatus::AVAILABLE;
}

DiagnosticStatus toDiagnosticStatus(FaultConfigurationError error);
DiagnosticStatus toDiagnosticStatus(FaultManagerResult result);

#endif
