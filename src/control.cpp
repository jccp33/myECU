#include "../include/utils.hpp"
#include "../include/control.hpp"
#include <cstddef>
#include <iostream>
#include <string>

Control::Control(std::size_t maxInvalidSignals)
        : state(EcuState::INIT), errors(0), shutdownRequested(false),
            maxInvalidSignals(maxInvalidSignals) {}

void Control::reset() {
    state = EcuState::INIT;
    errors = 0;
    signals.clear();
    shutdownRequested = false;
}

bool Control::allSignalsValid() {
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

void Control::processMessage(Message &mssg) {
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

void Control::printCurrentState() {
    std::string stateName = "UNDEFINED";
    std::string stateColor = TXT_RESET;
    switch (state) {
        case EcuState::INIT:
            stateName = "INIT";
            stateColor = TXT_BLUE;
            break;
        case EcuState::SELF_TEST:
            stateName = "SELF_TEST";
            stateColor = TXT_YELLOW;
            break;
        case EcuState::OPERATIONAL: 
            stateName = "OPERATIONAL";
            stateColor = TXT_GREEN;
            break;
        case EcuState::DEGRADED:
            stateName = "DEGRADED"; 
            stateColor = TXT_YELLOW;
            break;
        case EcuState::SAFE_STATE:
            stateName = "SAFE_STATE";
            stateColor = TXT_RED;
            break;
        case EcuState::SHUTDOWN:
            stateName = "SHUTDOWN";
            stateColor = TXT_BLUE;
            break;
    }
    std::cout << stateColor << "[CONTROL STATE]: " << stateName << TXT_RESET << std::endl;
}
