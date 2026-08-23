#include "../include/fault_manager.hpp"
#include "../include/fault_condition_evaluator.hpp"
#include "../include/fault_state_machine.hpp"
#include <cstddef>

namespace {

bool isActiveFaultState(FaultState state) {
    return state == FaultState::CONFIRMED
        || state == FaultState::RECOVERING
        || state == FaultState::LATCHED;
}

}

FaultManager::FaultManager(const EvaluationRule *confRules, std::size_t confRuleCount) : 
    rules(confRules),
    ruleCount(confRuleCount),
    records(),
    configurationValid(false),
    monitoringStartMs(0U),
    monitoringStarted(false)
{
    if (ruleCount > records.size()) return;
    if(ruleCount>0U && rules==nullptr) return;
    for(std::size_t i=0; i<ruleCount; i++){
        if(rules[i].validate() != RuleValidationError::NONE) return;
    }
    configurationValid = true;
}

std::size_t FaultManager::getRuleCount() const {
    return ruleCount;
}

bool FaultManager::isConfigValid() const {
    return configurationValid;
}

const FaultRecord *FaultManager::getRecord(std::size_t ruleIndex) const {
    if (!configurationValid || ruleIndex >= ruleCount) return nullptr;
    return &records[ruleIndex];
}

void FaultManager::reset() {
    resetForIgnitionCycle();
}

void FaultManager::resetForIgnitionCycle() {
    if (!configurationValid) return;
    for (std::size_t index = 0; index < ruleCount; ++index) records[index] = FaultRecord();
    monitoringStartMs = 0U;
    monitoringStarted = false;
}

FaultManagerResult FaultManager::processRule(
    std::size_t ruleIndex,
    float value,
    TimestampMs lastUpdateMs,
    TimestampMs nowMs
) {
    if (!configurationValid) return FaultManagerResult::INVALID_CONFIGURATION;
    if (ruleIndex >= ruleCount) return FaultManagerResult::INVALID_RULE_INDEX;
    const ConditionEvaluationResult evaluation = evaluateFaultCondition(
        rules[ruleIndex],
        value,
        lastUpdateMs,
        nowMs
    );
    switch (evaluation) {
        case ConditionEvaluationResult::HEALTHY:
            records[ruleIndex] = updateFaultRecord(
                rules[ruleIndex],
                records[ruleIndex],
                false,
                nowMs
            );
            return FaultManagerResult::OK;
        case ConditionEvaluationResult::FAULT_ACTIVE:
            records[ruleIndex] = updateFaultRecord(
                rules[ruleIndex],
                records[ruleIndex],
                true,
                nowMs
            );
            return FaultManagerResult::OK;
        case ConditionEvaluationResult::CLOCK_ERROR:
            return FaultManagerResult::CLOCK_ERROR;
        case ConditionEvaluationResult::INVALID_RULE:
        case ConditionEvaluationResult::UNSUPPORTED_EVALUATION_TYPE:
            return FaultManagerResult::RULE_EVALUATION_ERROR;
    }
    return FaultManagerResult::RULE_EVALUATION_ERROR;
}

FaultSummary FaultManager::getSummary() const {
    FaultSummary summary;
    if(!configurationValid) return summary;
    for(std::size_t i=0U; i<ruleCount; i++){
        const FaultState state = records[i].state;
        if(!isActiveFaultState(state)) continue;
        ++summary.activeFaultCount;
        const FaultSeverity severity = rules[i].getSeverity();
        if(severity == FaultSeverity::DEGRADED) summary.hasDegraded = true;
        if(severity == FaultSeverity::CRITICAL){
            if(state == FaultState::LATCHED) summary.hasCriticalLatched = true;
            else summary.hasCriticalActive = true;
        }
    }
    return summary;
}

FaultManagerResult FaultManager::processSignal(const SignalSample &sample, TimestampMs nowMs){
    if(!configurationValid) return FaultManagerResult::INVALID_CONFIGURATION;
    if(sample.validity != SignalValidity::VALID) return FaultManagerResult::INVALID_SIGNAL_SAMPLE;
    bool matchingRuleFound = false;
    for(std::size_t i=0U; i<ruleCount; i++){
        if(rules[i].getSignal() != sample.id) continue;
        matchingRuleFound = true;
        const FaultManagerResult result = processRule(
            i, 
            sample.value, 
            sample.lastUpdateMs, 
            nowMs
        );
        if(result != FaultManagerResult::OK) return result;
    }
    if(!matchingRuleFound) return FaultManagerResult::NO_MATCHING_RULE;
    return FaultManagerResult::OK;
}

FaultManagerResult FaultManager::processCycle(const SignalStore &store, TimestampMs nowMs){
    if(!configurationValid) return FaultManagerResult::INVALID_CONFIGURATION;
    if(monitoringStarted && nowMs<monitoringStartMs) return FaultManagerResult::CLOCK_ERROR;
    for(std::size_t i=0U; i<ruleCount; i++){
        const SignalSample *sample = store.find(rules[i].getSignal());
        if(sample!=nullptr && nowMs<sample->lastUpdateMs) return FaultManagerResult::CLOCK_ERROR;
    }
    if(!monitoringStarted){
        monitoringStartMs = nowMs;
        monitoringStarted = true;
    }
    FaultManagerResult cycleResult = FaultManagerResult::OK;
    for(std::size_t i=0U; i<ruleCount; i++){
        const EvaluationRule &rule = rules[i];
        const SignalSample *sample = store.find(rule.getSignal());
        switch (rule.getEvaluationType()) {
            case EvaluationType::RANGE:
                if(sample == nullptr) continue;
                if(sample->validity != SignalValidity::VALID){
                    cycleResult = FaultManagerResult::INVALID_SIGNAL_SAMPLE;
                    continue;
                }
                {
                    const FaultManagerResult result = processRule(
                        i, 
                        sample->value, 
                        sample->lastUpdateMs, 
                        nowMs
                    );
                    if(result != FaultManagerResult::OK) return result;
                }
                break;
            case EvaluationType::TIMEOUT:
                {
                    const TimestampMs refTimeMs = sample != nullptr ? sample->lastUpdateMs : monitoringStartMs;
                    const float unusedValue = sample != nullptr ? sample->value : 0.0F;
                    const FaultManagerResult result = processRule(
                        i, 
                        unusedValue, 
                        refTimeMs, 
                        nowMs
                    );
                    if(result != FaultManagerResult::OK) return result;
                }
                break;
            case EvaluationType::RATE_OF_CHANGE:
                return FaultManagerResult::RULE_EVALUATION_ERROR;
        }
    }
    return cycleResult;
}
