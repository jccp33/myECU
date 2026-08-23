#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "data_types.hpp"

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
        TimestampMs timeoutMs;
        bool isShutdownRequest;
        float activeValue;
        TimestampMs timestampMs;    // milisegundos
    public: 
        // constructors
        Message();
        Message(
            uint32_t id, 
            SensorId sId, 
            float val, 
            bool critic,
            float min, 
            float max, 
            TimestampMs timeout, 
            bool shutdownRequest,
            float shutdownValue,
            TimestampMs timestamp
        );
        // setters
        void setMessageId(uint32_t id);
        void setSensorId(SensorId id);
        void setRawValue(float value);
        void setMinValue(float min);
        void setMaxValue(float value);
        void setSignalStatus(SignalStatus status);
        void setIsCritic(bool critic);
        void setTimesStamp(TimestampMs timestamp);
        // getters
        uint32_t getMessageId() const;
        SensorId getSensorId() const;
        float getRawValue() const;
        float getMinValue() const;
        float getMaxValue() const;
        SignalStatus getSignalStatus() const;
        bool getIsCritic() const;
        TimestampMs getTimeoutMs() const;
        bool getIsShutdownRequest() const;
        float getActiveValue() const;
        TimestampMs getTimestamp() const;
};

#endif
