#include "config.hpp"

namespace {
    constexpr std::size_t CONFIGURED_SENSOR_COUNT = 10U;
    static_assert(
        CONFIGURED_SENSOR_COUNT <= MAX_SENSOR_COUNT,
        "Configured sensors exceed fixed ECU capacity"
    );
}// namespace

SystemConfig getSystemConfig() {
    SystemConfig config = {};
    config.sensorCount = CONFIGURED_SENSOR_COUNT;
    config.sensors = {{
        {
            SignalId(1U, 1U, 100U, 0U),
            "Solicitud de Apagado",
            "-",
            0.0f,
            0.0f,
            1.0f,
            FaultSeverity::WARNING,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            true,
            false,
            1.0f
        },
        {
            SignalId(1U, 1U, 101U, 0U),
            "Solicitud de Freno",
            "-",
            0.0f,
            0.0f,
            1.0f,
            FaultSeverity::WARNING,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            true,
            1.0f
        },
        {
            SignalId(1U, 1U, 102U, 0U),
            "Velocidad",
            "km/h",
            0.0f,
            0.0f,
            220.0f,
            FaultSeverity::DEGRADED,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 103U, 0U),
            "Revoluciones X minuto",
            "RPM",
            0.0f,
            0.0f,
            7000.0f,
            FaultSeverity::CRITICAL,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 104U, 0U),
            "Temperatura",
            "C",
            25.0f,
            -20.0f,
            130.0f,
            FaultSeverity::CRITICAL,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 105U, 0U),
            "Voltaje",
            "V",
            12.5f,
            8.0f,
            16.0f,
            FaultSeverity::CRITICAL,
            500U,
            200U,
            500U,
            FaultLatching::LATCHED,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 106U, 0U),
            "Posicion de Mariposa",
            "V",
            0.0f,
            0.5f,
            4.8f,
            FaultSeverity::DEGRADED,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 107U, 0U),
            "Presion Absoluta",
            "V",
            0.0f,
            0.5f,
            4.7f,
            FaultSeverity::DEGRADED,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 108U, 0U),
            "Flujo de masa de aire",
            "g/s",
            0.0f,
            2.0f,
            120.0f,
            FaultSeverity::DEGRADED,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        },
        {
            SignalId(1U, 1U, 109U, 0U),
            "Sensor de Oxigeno",
            "V",
            0.0f,
            0.1f,
            0.9f,
            FaultSeverity::DEGRADED,
            500U,
            200U,
            500U,
            FaultLatching::RECOVERABLE,
            false,
            false,
            0.0f
        }
    }};
    return config;
}

bool isSystemConfigValid(const SystemConfig& config) {
    return config.sensorCount <= config.sensors.size();
}
