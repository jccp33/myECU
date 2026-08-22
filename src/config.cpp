#include "../include/config.hpp"

SystemConfig getSystemConfig() {
    SystemConfig config;
    config.maxInvalidSignals = 4;
    config.sensors = {
        {100, SensorId::SHUT_REQ, "Solicitud de Apagado",  "S_R",  0.0f,  0.0f,   1.0f,    false, 500, true,  1.0f},
        {101, SensorId::BRAKE,    "Solicitud de Freno",    "BRK",  0.0f,  0.0f,   1.0f,    false, 500, false, 1.0f},
        {102, SensorId::SPEED,    "Velocidad",             "km/h", 0.0f,  0.0f,   220.0f,  false, 500, false, 0.0f},
        {103, SensorId::RPM,      "Revoluciones X minuto", "RPM",  0.0f,  0.0f,   7000.0f, true,  500, false, 0.0f},
        {104, SensorId::TEMP,     "Temperatura",           "C",    25.0f, -20.0f, 130.0f,  true,  500, false, 0.0f},
        {105, SensorId::VOLTAGE,  "Voltaje",               "V",    12.5f,  8.0f,  16.0f,   true,  500, false, 0.0f},
        {106, SensorId::TPS,      "Posicion de Mariposa",  "V",    0.0f,  0.5f,   4.8f,    false, 500, false, 0.0f},
        {107, SensorId::MAP,      "Presion Absoluta",      "V",    0.0f,  0.5f,   4.7f,    false, 500, false, 0.0f},
        {108, SensorId::MAF,      "Flujo de masa de aire", "g/s",  0.0f,  2.0f,   120.0f,  false, 500, false, 0.0f},
        {109, SensorId::O2,       "Sensor de Oxigeno",     "V",    0.0f,  0.1f,   0.9f,    false, 500, false, 0.0f}
    };
    return config;
}
