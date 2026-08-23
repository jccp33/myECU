#include "../include/config.hpp"

namespace {

constexpr std::size_t CONFIGURED_SENSOR_COUNT = 10U;

static_assert(
    CONFIGURED_SENSOR_COUNT <= MAX_SENSOR_COUNT,
    "Configured sensors exceed fixed ECU capacity"
);

}  // namespace

SystemConfig getSystemConfig() {
    SystemConfig config = {};
    config.sensorCount = CONFIGURED_SENSOR_COUNT;
    config.maxInvalidSignals = 4;
    config.sensors = {{
        {100, SensorId::SHUT_REQ, SignalId(1U, 1U, 100U, 0U), "Solicitud de Apagado",  "S_R",  0.0f,  0.0f,   1.0f,    false, 500, true,  1.0f},
        {101, SensorId::BRAKE,    SignalId(1U, 1U, 101U, 0U), "Solicitud de Freno",    "BRK",  0.0f,  0.0f,   1.0f,    false, 500, false, 1.0f},
        {102, SensorId::SPEED,    SignalId(1U, 1U, 102U, 0U), "Velocidad",             "km/h", 0.0f,  0.0f,   220.0f,  false, 500, false, 0.0f},
        {103, SensorId::RPM,      SignalId(1U, 1U, 103U, 0U), "Revoluciones X minuto", "RPM",  0.0f,  0.0f,   7000.0f, true,  500, false, 0.0f},
        {104, SensorId::TEMP,     SignalId(1U, 1U, 104U, 0U), "Temperatura",           "C",    25.0f, -20.0f, 130.0f,  true,  500, false, 0.0f},
        {105, SensorId::VOLTAGE,  SignalId(1U, 1U, 105U, 0U), "Voltaje",               "V",    12.5f,  8.0f,  16.0f,   true,  500, false, 0.0f},
        {106, SensorId::TPS,      SignalId(1U, 1U, 106U, 0U), "Posicion de Mariposa",  "V",    0.0f,  0.5f,   4.8f,    false, 500, false, 0.0f},
        {107, SensorId::MAP,      SignalId(1U, 1U, 107U, 0U), "Presion Absoluta",      "V",    0.0f,  0.5f,   4.7f,    false, 500, false, 0.0f},
        {108, SensorId::MAF,      SignalId(1U, 1U, 108U, 0U), "Flujo de masa de aire", "g/s",  0.0f,  2.0f,   120.0f,  false, 500, false, 0.0f},
        {109, SensorId::O2,       SignalId(1U, 1U, 109U, 0U), "Sensor de Oxigeno",     "V",    0.0f,  0.1f,   0.9f,    false, 500, false, 0.0f}
    }};
    return config;
}

bool isSystemConfigValid(const SystemConfig& config) {
    return config.sensorCount <= config.sensors.size() && config.maxInvalidSignals <= config.sensorCount;
}
