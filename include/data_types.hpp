#ifndef DATA_TYPES_HPP
#define DATA_TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <array>

// Fixed capacity for the set of signals supervised by this ECU Core. The
// current configuration uses 10 entries; 128 stays within the 100-250 signal
// budget documented for a small 112 KiB MCU while leaving deterministic RAM.
constexpr std::size_t MAX_SENSOR_COUNT = 128U;

// enums 
enum class SensorId : uint8_t {
    SHUT_REQ,  // solicitud de apagado
    SPEED,     // velocidad (km/h)
    RPM,       // revoluciones por minuto
    TEMP,      // temperatura (°C)
    VOLTAGE,   // voltaje (V)
    BRAKE,     // freno
    TPS,       // Posición de la Mariposa del Acelerador
    MAP,       // Presión Absoluta del Múltiple de Admisión
    MAF,       // Flujo de Masa de Aire
    O2,        // Sensor de Oxígeno (Sonda Lambda - Convencional de Zirconio)
    UNDEFINED  // indefinido
};

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

enum class EvaluationType : std::uint8_t {
    RANGE = 0,
    TIMEOUT,
    RATE_OF_CHANGE,
};

enum class FaultLatching : std::uint8_t {
    RECOVERABLE = 0,
    LATCHED,
};

enum class RuleValidationError : std::uint8_t {
    NONE = 0,
    INVALID_EVALUATION_TYPE,
    INVALID_THRESHOLDS,
    ERROR_TYPE_MISMATCH,
    INVALID_SEVERITY,
    INVALID_MAXIMUM_AGE
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

struct InitValues {
    uint32_t id;      // message id
    SensorId sId;     // sensor id
    SignalId signalId;
    const char* name;
    const char* unit;
    float value;      // value
    float minValue; 
    float maxValue;
    bool isCritic;
    uint64_t timeoutMs;
    bool isShutdownRequest;
    float activeValue;
};

struct SystemConfig {
    std::array<InitValues, MAX_SENSOR_COUNT> sensors;
    std::size_t sensorCount;
    std::size_t maxInvalidSignals;
};

// types
using TimestampMs = std::uint64_t;

#endif
