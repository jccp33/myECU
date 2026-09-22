#include "data_types.hpp"
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
    const SignalId id(1U, 10U, 100U, 0U);
    const SignalId same(1U, 10U, 100U, 0U);
    const SignalId differentInstance(1U, 10U, 100U, 1U);
    const FaultSummary healthy;
    const FaultSummary active(true, false, true, 2U);
    const bool passed =
        expectTrue("SignalId equality uses every field", id == same && id != differentInstance)
        && expectTrue("default summary is healthy", !healthy.hasActiveFaults())
        && expectTrue("summary reports active faults", active.hasCriticalActive && active.hasDegraded && active.hasActiveFaults())
        && expectTrue("domain enums use fixed byte storage",
            sizeof(SignalStatus) == sizeof(std::uint8_t)
            && sizeof(FaultSeverity) == sizeof(std::uint8_t)
            && sizeof(FaultState) == sizeof(std::uint8_t)
            && sizeof(FaultLatching) == sizeof(std::uint8_t))
        && expectTrue("TimestampMs is unsigned 64 bit", std::is_same<TimestampMs, std::uint64_t>::value);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
