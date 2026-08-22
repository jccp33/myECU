#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <string>

// Sensores
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

// Estados (evaluado por la Gateway ECU)
enum class SignalStatus : uint8_t {
    VALID = 0,
    OUT_OF_RANGE = 1,
    TIMEOUT = 2,
    UNDEFINED = 3  // indefinido
};

// Mensaje
class Message {
    private:
        uint32_t messageId;      // identificador
        SensorId sensorId;       // sensor/señal
        float rawValue;          // valor crudo
        float minValue;
        float maxValue;
        SignalStatus status;     // determinado por Gateway
        bool isCritic;           // es sensor critico (true, false)
        uint64_t timestampMs;    // milisegundos
    public: 
        // constructors
        Message();
        Message(uint32_t id, SensorId sId, float val, bool critic, float min, float max);
        // setters
        void setMessageId(uint32_t id);
        void setSensorId(SensorId id);
        void setRawValue(float value);
        void setMinValue(float min);
        void setMaxValue(float value);
        void setSignalStatus(SignalStatus status);
        void setIsCritic(bool critic);
        void setTimesStamp(uint64_t timestamp);
        // getters
        uint32_t getMessageId();
        SensorId getSensorId();
        float getRawValue();
        float getMinValue();
        float getMaxValue();
        SignalStatus getSignalStatus();
        bool getIsCritic();
        uint64_t getTimestamp();
        const char* getUnit();
        const char* getName();
        // Methods
        std::string getMessageString();
        std::string getStdMessageString();
        std::string getStdColorsMessageString();
};

#endif
