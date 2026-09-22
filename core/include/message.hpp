#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "data_types.hpp"

// Mensaje
class Message {
    private:
        SignalId signalId;
        float rawValue;
        float minValue;
        float maxValue;
        SignalStatus status;
        TimestampMs timeoutMs;
        bool isShutdownRequest;
        bool isShutdownPermission;
        float activeValue;
        TimestampMs timestampMs;
    public:
        Message();
        Message(
            const SignalId &sigId,
            float val,
            float min,
            float max,
            TimestampMs timeout,
            bool shutdownRequest,
            bool shutdownPermission,
            float shutdownValue,
            TimestampMs timestamp
        );
        // setters
        void setRawValue(float value);
        void setSignalStatus(SignalStatus status);
        void setTimesStamp(TimestampMs timestamp);
        // getters
        const SignalId &getSignalId() const;
        float getRawValue() const;
        float getMinValue() const;
        float getMaxValue() const;
        SignalStatus getSignalStatus() const;
        TimestampMs getTimeoutMs() const;
        bool getIsShutdownRequest() const;
        bool getIsShutdownPermission() const;
        float getActiveValue() const;
        TimestampMs getTimestamp() const;
};

#endif
