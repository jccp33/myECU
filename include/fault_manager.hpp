#ifndef FAULT_MANAGER_HPP
#define FAULT_MANAGER_HPP

#include "data_types.hpp"
#include "evaluation_rule.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

static_assert(
    MAX_SENSOR_COUNT <= std::numeric_limits<std::uint16_t>::max(),
    "FaultSummary::activeFaultCount cannot represent every configured signal"
);

enum class FaultManagerResult : std::uint8_t {
    OK = 0,
    INVALID_CONFIGURATION,
    SIGNAL_NOT_CONFIGURED,
    CLOCK_ERROR
};

class FaultManager {
    private:
        const EvaluationRule* rules;
        std::size_t ruleCount;
        std::array<FaultRecord, MAX_SENSOR_COUNT> records;
        bool configurationValid;
    public:
        FaultManager(const EvaluationRule* confRules, std::size_t confRuleCount);
        FaultManagerResult processCondition(
            const SignalId &signalId,
            bool faultConditionActive,
            TimestampMs nowMs
        );
        const FaultRecord *getRecord(const SignalId &signalId) const;
        std::size_t getRuleCount() const;
        bool isConfigValid() const;
        void reset();
        void resetForIgnitionCycle();
        FaultSummary getSummary() const;
};

#endif
