#ifndef SIGNAL_STORE_HPP
#define SIGNAL_STORE_HPP

#include "data_types.hpp"
#include "signal_sample.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

enum class SignalStoreResult : std::uint8_t {
    INSERTED = 0,
    UPDATED,
    STALE_SAMPLE,
    CAPACITY_EXCEEDED
};

class SignalStore{
    private:
        std::array<SignalSample, MAX_SENSOR_COUNT> samples;
        std::size_t sampleCount;
    public:
        SignalStore();
        SignalStoreResult upsert(const SignalSample &sample);
        const SignalSample *find(const SignalId &id) const;
        const SignalSample *get(std::size_t index) const;
        std::size_t size() const;
        std::size_t capacity() const { return MAX_SENSOR_COUNT; }
        void clear();
};

#endif
