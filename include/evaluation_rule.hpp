#ifndef EVALUATION_RULE_HPP
#define EVALUATION_RULE_HPP

#include "data_types.hpp"

class EvaluationRule {
    private:
        SignalId signal;
        TimestampMs confirmationTimeMs;
        TimestampMs recoveryTimeMs;
        FaultSeverity severity;
        FaultLatching latching;
    public:
        constexpr EvaluationRule(
            const SignalId& signalId = SignalId(),
            TimestampMs confirmationTime = 0U,
            TimestampMs recoveryTime = 0U,
            FaultSeverity faultSeverity = FaultSeverity::NONE,
            FaultLatching faultLatching = FaultLatching::RECOVERABLE
        ) :
            signal(signalId),
            confirmationTimeMs(confirmationTime),
            recoveryTimeMs(recoveryTime),
            severity(faultSeverity),
            latching(faultLatching) {}
        constexpr const SignalId& getSignalId() const { return signal; }
        constexpr TimestampMs getConfirmationTimeMs() const { return confirmationTimeMs; }
        constexpr TimestampMs getRecoveryTimeMs() const { return recoveryTimeMs; }
        constexpr FaultSeverity getSeverity() const { return severity; }
        constexpr FaultLatching getLatching() const { return latching; }
        bool isValid() const;
};

#endif
