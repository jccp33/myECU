#ifndef FAULT_MANAGER_HPP
#define FAULT_MANAGER_HPP

#include "data_types.hpp"
#include "evaluation_rule.hpp"
#include "fault_types.hpp"
#include "signal_sample.hpp"
#include "signal_store.hpp"
#include "fault_configuration.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

static_assert(
    MAX_EVALUATION_RULE_COUNT <= std::numeric_limits<std::uint16_t>::max(),
    "FaultSummary::activeFaultCount cannot represent every configured rule"
);

enum class FaultManagerResult : std::uint8_t {
    OK = 0,
    INVALID_CONFIGURATION,
    INVALID_RULE_INDEX,
    INVALID_SIGNAL_SAMPLE,
    NO_MATCHING_RULE,
    RULE_EVALUATION_ERROR,
    CLOCK_ERROR
};

class FaultManager{
    private:
        const EvaluationRule *rules;
        std::size_t ruleCount;
        std::array<FaultRecord, MAX_EVALUATION_RULE_COUNT> records;
        bool configurationValid;
        TimestampMs monitoringStartMs;
        bool monitoringStarted;
    public:
        FaultManager(const EvaluationRule *confRules, std::size_t confRuleCount);
        FaultManagerResult processRule(
            std::size_t ruleIndex,
            float value,
            TimestampMs lastUpdateMs,
            TimestampMs nowMs
        ); 
        FaultManagerResult processSignal(
            const SignalSample &sample,
            TimestampMs nowMs
        );
        FaultManagerResult processCycle(
            const SignalStore& store,
            TimestampMs nowMs
        );
        const FaultRecord *getRecord(std::size_t ruleIndex) const;
        std::size_t getRuleCount() const;
        bool isConfigValid() const;
        void reset();
        void resetForIgnitionCycle();
        FaultSummary getSummary() const;
};

#endif
