#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "data_types.hpp"

// Mensaje
class Message {
    private:
        uint32_t messageId;
        SensorId sensorId;
        SignalId signalId;
        float rawValue;
        float minValue;
        float maxValue;
        SignalStatus status;
        FaultSeverity severity;
        TimestampMs timeoutMs;
        bool isShutdownRequest;
        float activeValue;
        TimestampMs timestampMs;
    public:
        Message();
        Message(
            uint32_t id,
            SensorId sId,
            const SignalId &sigId,
            float val,
            FaultSeverity _severity,
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
        void setSignalId(const SignalId &id);
        void setRawValue(float value);
        void setMinValue(float min);
        void setMaxValue(float value);
        void setSignalStatus(SignalStatus status);
        void setSeverity(FaultSeverity _severity);
        void setTimesStamp(TimestampMs timestamp);
        // getters
        uint32_t getMessageId() const;
        SensorId getSensorId() const;
        const SignalId &getSignalId() const;
        float getRawValue() const;
        float getMinValue() const;
        float getMaxValue() const;
        SignalStatus getSignalStatus() const;
        FaultSeverity getSeverity() const;
        TimestampMs getTimeoutMs() const;
        bool getIsShutdownRequest() const;
        float getActiveValue() const;
        TimestampMs getTimestamp() const;
};

#endif
