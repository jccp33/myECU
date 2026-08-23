#include "../include/signal_store.hpp"
#include <cstddef>

SignalStore::SignalStore() : 
    samples(),
    sampleCount(0U) {}

SignalStoreResult SignalStore::upsert(const SignalSample &sample){
    for(std::size_t i=0U; i<sampleCount; i++){
        if(samples[i].id != sample.id) continue;
        if(sample.lastUpdateMs < samples[i].lastUpdateMs) return SignalStoreResult::STALE_SAMPLE;
        samples[i] = sample;
        return SignalStoreResult::UPDATED;
    }
    if(sampleCount >= samples.size()) return SignalStoreResult::CAPACITY_EXCEEDED;
    samples[sampleCount] = sample;
    ++sampleCount;
    return SignalStoreResult::INSERTED;
}

const SignalSample *SignalStore::find(const SignalId &id) const {
    for(std::size_t i=0U; i<sampleCount; i++){
        if(samples[i].id == id){
            return &samples[i];
        }
    }
    return nullptr;
}

const SignalSample* SignalStore::get(std::size_t index) const {
    if (index >= sampleCount) {
        return nullptr;
    }
    return &samples[index];
}

std::size_t SignalStore::size() const {
    return sampleCount;
}

void SignalStore::clear() {
    sampleCount = 0U;
}
