#ifndef FAULT_TYPES_HPP
#define FAULT_TYPES_HPP

#include "data_types.hpp"

enum class SignalError : std::uint8_t {
    NONE = 0,
    TIMEOUT,
    OUT_OF_RANGE,
    RATE_OF_CHANGE,
    SENSOR_FAILURE,
    COMMUNICATION_FAILURE,
    INVALID_DATA
};

enum class FaultSeverity : std::uint8_t {
    NONE = 0,
    WARNING,
    DEGRADED,
    CRITICAL
};

enum class FaultType : std::uint8_t {
    SENSOR = 0,
    ACTUATOR,
    COMMUNICATION,
    INTERNAL,
    CONFIGURATION
};

struct FaultSummary {
    bool hasCriticalActive;
    bool hasCriticalLatched;
    bool hasDegraded;
    std::uint16_t activeFaultCount;
    constexpr FaultSummary(
        bool criticalActive = false,
        bool criticalLatched = false,
        bool degraded = false,
        std::uint16_t faultCount = 0U
    ) :
        hasCriticalActive(criticalActive),
        hasCriticalLatched(criticalLatched),
        hasDegraded(degraded),
        activeFaultCount(faultCount) {}
    constexpr bool hasActiveFaults() const {
        return activeFaultCount != 0U;
    }
};

enum class FaultState : std::uint8_t {
    INACTIVE = 0,
    PENDING,
    CONFIRMED,
    RECOVERING,
    LATCHED
};

struct FaultRecord {
    FaultState state;
    TimestampMs stateEntryTimeMs;
    constexpr FaultRecord(
        FaultState initialState = FaultState::INACTIVE,
        TimestampMs entryTimeMs = 0U
    ) :
        state(initialState),
        stateEntryTimeMs(entryTimeMs) {}
};

#endif
