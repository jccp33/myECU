#include "fault_manager.hpp"
#include "fault_state_machine.hpp"
#include <cstddef>

namespace {
    bool isActiveFaultState(FaultState state) {
        return state == FaultState::CONFIRMED
            || state == FaultState::RECOVERING
            || state == FaultState::LATCHED;
    }
    bool hasClockRegression(const FaultRecord& record, TimestampMs nowMs) {
        return record.state != FaultState::INACTIVE
            && nowMs < record.stateEntryTimeMs;
    }
} // namespace


FaultManager::FaultManager(const EvaluationRule* confRules, std::size_t confRuleCount) :
    rules(confRules),
    ruleCount(confRuleCount),
    records(),
    configurationValid(false)
{
    if (ruleCount > records.size()) return;
    if (ruleCount > 0U && rules == nullptr) return;
    for (std::size_t i = 0U; i < ruleCount; ++i) {
        if (!rules[i].isValid()) return;
    }
    configurationValid = true;
}

std::size_t FaultManager::getRuleCount() const { return ruleCount; }

bool FaultManager::isConfigValid() const { return configurationValid; }

const FaultRecord* FaultManager::getRecord(const SignalId& signalId) const {
    if (!configurationValid) return nullptr;
    for (std::size_t i = 0U; i < ruleCount; ++i) {
        if (rules[i].getSignalId() == signalId) return &records[i];
    }
    return nullptr;
}

void FaultManager::reset() { resetForIgnitionCycle(); }

void FaultManager::resetForIgnitionCycle() {
    if (!configurationValid) return;
    for (std::size_t i = 0U; i < ruleCount; ++i) {
        records[i] = FaultRecord();
    }
}

FaultManagerResult FaultManager::processCondition(
    const SignalId& signalId, 
    bool faultConditionActive, 
    TimestampMs nowMs
) {
    if (!configurationValid) return FaultManagerResult::INVALID_CONFIGURATION;
    for (std::size_t i = 0U; i < ruleCount; ++i) {
        if (rules[i].getSignalId() != signalId) continue;
        if (hasClockRegression(records[i], nowMs)) return FaultManagerResult::CLOCK_ERROR;
        records[i] = updateFaultRecord(rules[i], records[i], faultConditionActive, nowMs);
        return FaultManagerResult::OK;
    }
    return FaultManagerResult::SIGNAL_NOT_CONFIGURED;
}

FaultSummary FaultManager::getSummary() const {
    FaultSummary summary;
    if (!configurationValid) return summary;
    for (std::size_t i = 0U; i < ruleCount; ++i) {
        const FaultState state = records[i].state;
        if (!isActiveFaultState(state)) continue;
        ++summary.activeFaultCount;
        const FaultSeverity severity = rules[i].getSeverity();
        if (severity == FaultSeverity::DEGRADED) summary.hasDegraded = true;
        if (severity == FaultSeverity::CRITICAL) {
            if (state == FaultState::LATCHED) {
                summary.hasCriticalLatched = true;
            } else {
                summary.hasCriticalActive = true;
            }
        }
    }
    return summary;
}
