#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <array>

#ifndef MYECU_MAX_SENSOR_COUNT
#define MYECU_MAX_SENSOR_COUNT 128U
#endif

using TimestampMs = std::uint64_t;
constexpr std::size_t MAX_SENSOR_COUNT = static_cast<std::size_t>(MYECU_MAX_SENSOR_COUNT);

enum class SignalStatus : uint8_t {
    VALID = 0,
    OUT_OF_RANGE = 1,
    TIMEOUT = 2,
    UNDEFINED = 3  // indefinido
};

enum class EcuState : std::uint8_t {
    INIT = 0,
    SELF_TEST = 1,
    OPERATIONAL = 2,
    DEGRADED = 3,
    SAFE_STATE = 4,
    SHUTDOWN_REQ = 5,
    SHUTDOWN = 6
};

enum class FaultLatching : std::uint8_t {
    RECOVERABLE = 0,
    LATCHED,
};

// structs
struct SignalId {
    std::uint16_t ecu;
    std::uint16_t source;
    std::uint16_t id;
    std::uint16_t instance;
    constexpr SignalId(
        std::uint16_t ecuValue = 0U,
        std::uint16_t sourceValue = 0U,
        std::uint16_t idValue = 0U,
        std::uint16_t instanceValue = 0U
    ) :
        ecu(ecuValue),
        source(sourceValue),
        id(idValue),
        instance(instanceValue) {}
};

constexpr bool operator==(const SignalId& left, const SignalId& right) {
    return left.ecu == right.ecu
        && left.source == right.source
        && left.id == right.id
        && left.instance == right.instance;
}

constexpr bool operator!=(const SignalId& left, const SignalId& right) {
    return !(left == right);
}

enum class FaultSeverity : std::uint8_t {
    NONE = 0,
    WARNING,
    DEGRADED,
    CRITICAL
};

enum class FaultState : std::uint8_t {
    INACTIVE = 0,
    PENDING,
    CONFIRMED,
    RECOVERING,
    LATCHED
};

struct InitValues {
    SignalId signalId;
    const char* name;
    const char* unit;
    float value;
    float minValue;
    float maxValue;
    FaultSeverity severity;
    TimestampMs timeoutMs;
    TimestampMs confirmationTimeMs;
    TimestampMs recoveryTimeMs;
    FaultLatching latching;
    bool isShutdownRequest;
    bool isShutdownPermission;
    float activeValue;
};

struct SystemConfig {
    std::array<InitValues, MAX_SENSOR_COUNT> sensors;
    std::size_t sensorCount;
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
