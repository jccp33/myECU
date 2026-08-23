#include "../include/fault_state_machine.hpp"

namespace {

FaultState getConfirmedState(const EvaluationRule &rule){
    if(rule.getLatching() == FaultLatching::LATCHED){
        return FaultState::LATCHED;
    }
    return FaultState::CONFIRMED;
}

FaultRecord createConfirmedRecord(const EvaluationRule &rule, TimestampMs nowMs){
    return FaultRecord(getConfirmedState(rule), nowMs);
}

}

FaultRecord updateFaultRecord(
    const EvaluationRule &rule,
    const FaultRecord &current,
    bool faultConditionActive,
    TimestampMs nowMs
){
    switch(current.state){
        case FaultState::INACTIVE:
            if(!faultConditionActive) return current;
            if(rule.getConfirmationTimeMs() == 0U) return createConfirmedRecord(rule, nowMs);
            return FaultRecord(FaultState::PENDING, nowMs);
        case FaultState::PENDING:
            if(!faultConditionActive) return FaultRecord(FaultState::INACTIVE, nowMs);
            if(nowMs < current.stateEntryTimeMs) return FaultRecord(FaultState::PENDING, nowMs);
            if((nowMs-current.stateEntryTimeMs) < rule.getConfirmationTimeMs()) return current;
            return createConfirmedRecord(rule, nowMs);
        case FaultState::CONFIRMED:
            if(faultConditionActive) return current;
            if(rule.getRecoveryTimeMs() == 0U) return FaultRecord(FaultState::INACTIVE, nowMs);
            return FaultRecord(FaultState::RECOVERING, nowMs);
        case FaultState::RECOVERING:
            if(faultConditionActive) return FaultRecord(FaultState::CONFIRMED, nowMs);
            if(nowMs < current.stateEntryTimeMs) return FaultRecord(FaultState::RECOVERING, nowMs);
            if((nowMs-current.stateEntryTimeMs) < rule.getRecoveryTimeMs()) return current;
            return FaultRecord(FaultState::INACTIVE, nowMs);
        case FaultState::LATCHED:
            return current;
    }
    return current;
}
