#ifndef SIGNAL_SAMPLE_HPP
#define SIGNAL_SAMPLE_HPP

#include "data_types.hpp"
#include <cstdint>

enum class SignalValidity : std::uint8_t {
    UNKNOWN = 0,
    VALID,
    INVALID
};

struct SignalSample {
    SignalId id;
    float value;
    TimestampMs lastUpdateMs;
    SignalValidity validity;
    constexpr SignalSample(
        const SignalId &signalId = SignalId(),
        float signalValue = 0.0F,
        TimestampMs updateTimeMs = 0U,
        SignalValidity signalValidity = SignalValidity::UNKNOWN
    ) : 
        id(signalId),
        value(signalValue),
        lastUpdateMs(updateTimeMs),
        validity(signalValidity) {}
};

#endif
