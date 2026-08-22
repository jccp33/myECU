#include "../include/control.hpp"
#include <cstddef>

Control::Control(std::size_t invalidSignalLimit) : 
    state(EcuState::INIT), errors(0), 
    shutdownRequested(false), 
    maxInvalidSignals(invalidSignalLimit) {}

void Control::reset() {
    state = EcuState::INIT;
    errors = 0;
    signals.clear();
    shutdownRequested = false;
}

bool Control::allSignalsValid() const {
    for (std::size_t signal = 0; signal < signals.size(); signal++) {
        if (signals[signal].status != SignalStatus::VALID) {
            return false;
        }
    }
    return true;
}

void Control::updateState() {
    std::size_t invalidSignals = 0;
    bool criticalFailure = false;
    for (std::size_t signal = 0; signal < signals.size(); signal++) {
        if (signals[signal].status != SignalStatus::VALID) {
            invalidSignals++;
            if (signals[signal].critical) {
                criticalFailure = true;
            }
        }
    }
    if (allSignalsValid()) {
        state = EcuState::OPERATIONAL;
    } else if (criticalFailure || invalidSignals > maxInvalidSignals) {
        state = EcuState::SAFE_STATE;
    } else {
        state = EcuState::DEGRADED;
    }
}

void Control::processMessage(const Message &mssg) {
    SignalRecord* record = 0;
    for (std::size_t signal = 0; signal < signals.size(); signal++) {
        if (signals[signal].messageId == mssg.getMessageId()) {
            record = &signals[signal];
            break;
        }
    }
    if (record == 0) {
        SignalRecord newRecord = {mssg.getMessageId(), SignalStatus::UNDEFINED, false};
        signals.push_back(newRecord);
        record = &signals.back();
    }
    record->status = mssg.getSignalStatus();
    record->critical = mssg.getIsCritic();
    if (mssg.getIsShutdownRequest() && mssg.getRawValue() == mssg.getActiveValue()) {
        shutdownRequested = true;
    }
    if (state == EcuState::SHUTDOWN) {
        return;
    }
    if (state == EcuState::INIT) {
        errors = 0;
        state = EcuState::SELF_TEST;
        return;
    }
    updateState();
    if (shutdownRequested && (state == EcuState::DEGRADED || state == EcuState::SAFE_STATE)) {
        state = EcuState::SHUTDOWN;
    }
}
