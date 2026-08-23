#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }
    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testConfigurationResultMapping() {
    return expectTrue(
        "configuration results map to diagnostic status",
        toDiagnosticStatus(FaultConfigurationError::NONE)
                == DiagnosticStatus::AVAILABLE
            && toDiagnosticStatus(FaultConfigurationError::NULL_RULES)
                == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultConfigurationError::UNKNOWN_SIGNAL)
                == DiagnosticStatus::CONFIGURATION_ERROR
    );
}

bool testManagerResultMapping() {
    return expectTrue(
        "fault manager results map to diagnostic status",
        toDiagnosticStatus(FaultManagerResult::OK)
                == DiagnosticStatus::AVAILABLE
            && toDiagnosticStatus(FaultManagerResult::NO_MATCHING_RULE)
                == DiagnosticStatus::AVAILABLE
            && toDiagnosticStatus(FaultManagerResult::INVALID_CONFIGURATION)
                == DiagnosticStatus::CONFIGURATION_ERROR
            && toDiagnosticStatus(FaultManagerResult::CLOCK_ERROR)
                == DiagnosticStatus::CLOCK_ERROR
            && toDiagnosticStatus(FaultManagerResult::INVALID_RULE_INDEX)
                == DiagnosticStatus::EVALUATION_ERROR
            && toDiagnosticStatus(FaultManagerResult::RULE_EVALUATION_ERROR)
                == DiagnosticStatus::EVALUATION_ERROR
            && toDiagnosticStatus(FaultManagerResult::INVALID_SIGNAL_SAMPLE)
                == DiagnosticStatus::INVALID_SIGNAL_SAMPLE
    );
}

bool testAvailabilityPredicate() {
    return expectTrue(
        "only AVAILABLE diagnostic status is available",
        isDiagnosticAvailable(DiagnosticStatus::AVAILABLE)
            && !isDiagnosticAvailable(DiagnosticStatus::NOT_AVAILABLE)
            && !isDiagnosticAvailable(DiagnosticStatus::CLOCK_ERROR)
            && !isDiagnosticAvailable(DiagnosticStatus::EVALUATION_ERROR)
    );
}

bool testDiagnosticStatusUsesFixedWidthStorage() {
    return expectTrue(
        "DiagnosticStatus uses uint8_t storage",
        std::is_same<
            std::underlying_type<DiagnosticStatus>::type,
            std::uint8_t
        >::value
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testConfigurationResultMapping(),
        testManagerResultMapping(),
        testAvailabilityPredicate(),
        testDiagnosticStatusUsesFixedWidthStorage()
    };

    const std::size_t count = sizeof(results) / sizeof(results[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        if (!results[index]) {
            ++failures;
        }
    }

    if (failures != 0) {
        return EXIT_FAILURE;
    }
    std::cout << "All diagnostic status tests passed\n";
    return EXIT_SUCCESS;
}
