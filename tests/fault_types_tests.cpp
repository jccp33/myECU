#include "../include/fault_types.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }
    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testSignalIdentityUsesEveryField() {
    const SignalId rpmPrimary(1U, 10U, 100U, 0U);
    const SignalId sameRpm(1U, 10U, 100U, 0U);
    const SignalId redundantRpm(1U, 10U, 100U, 1U);
    return expectTrue(
        "SignalId equality uses ECU, source, id and instance",
        rpmPrimary == sameRpm && rpmPrimary != redundantRpm
    );
}

bool testDefaultFaultSummaryIsHealthy() {
    const FaultSummary summary;
    return expectTrue(
        "default FaultSummary represents no active faults",
        !summary.hasCriticalActive
            && !summary.hasCriticalLatched
            && !summary.hasDegraded
            && !summary.hasActiveFaults()
    );
}

bool testFaultSummaryReportsActiveFaults() {
    const FaultSummary summary(true, false, true, 3U);
    return expectTrue(
        "FaultSummary preserves aggregate fault information",
        summary.hasCriticalActive
            && !summary.hasCriticalLatched
            && summary.hasDegraded
            && summary.activeFaultCount == 3U
            && summary.hasActiveFaults()
    );
}

bool testFaultEnumsHaveFixedWidthStorage() {
    const bool fixedWidth =
        std::is_same<
            std::underlying_type<SignalError>::type,
            std::uint8_t
        >::value
        && std::is_same<
            std::underlying_type<FaultSeverity>::type,
            std::uint8_t
        >::value
        && std::is_same<
            std::underlying_type<FaultType>::type,
            std::uint8_t
        >::value
        && std::is_same<
            std::underlying_type<FaultState>::type,
            std::uint8_t
        >::value;
    return expectTrue("fault enums use explicit uint8_t storage", fixedWidth);
}

bool testDefaultFaultRecordIsInactive() {
    const FaultRecord record;

    return expectTrue(
        "default FaultRecord is inactive",
        record.state == FaultState::INACTIVE
            && record.stateEntryTimeMs == 0U
    );
}

bool testFaultRecordPreservesExplicitState() {
    const FaultRecord record(
        FaultState::PENDING,
        1000U
    );

    return expectTrue(
        "FaultRecord preserves explicit state and entry time",
        record.state == FaultState::PENDING
            && record.stateEntryTimeMs == 1000U
    );
}

int main() {
    int failures = 0;
    const bool results[] = {
        testSignalIdentityUsesEveryField(),
        testDefaultFaultSummaryIsHealthy(),
        testFaultSummaryReportsActiveFaults(),
        testFaultEnumsHaveFixedWidthStorage(),
        testDefaultFaultRecordIsInactive(),
        testFaultRecordPreservesExplicitState()
    };
    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " fault type test(s) failed\n";
        return EXIT_FAILURE;
    }
    std::cout << "All fault type tests passed\n";
    return EXIT_SUCCESS;
}
