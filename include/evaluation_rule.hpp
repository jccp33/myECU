#ifndef EVALUATION_RULE_HPP
#define EVALUATION_RULE_HPP

#include "data_types.hpp"
#include "fault_types.hpp"

class EvaluationRule {
    public:
        constexpr EvaluationRule(
            const SignalId& signalId,
            EvaluationType evaluationType,
            float lower,
            float upper,
            TimestampMs confirmationTime,
            TimestampMs recoveryTime,
            TimestampMs maximumAge,
            SignalError signalError,
            FaultSeverity faultSeverity,
            FaultType type,
            FaultLatching faultLatching
        ) :
            signal(signalId),
            evalType(evaluationType),
            lowerThreshold(lower),
            upperThreshold(upper),
            confirmationTimeMs(confirmationTime),
            recoveryTimeMs(recoveryTime),
            maximumAgeMs(maximumAge),
            error(signalError),
            severity(faultSeverity),
            faultType(type),
            latching(faultLatching) {}

        RuleValidationError validate() const;
        constexpr const SignalId& getSignal() const { return signal; }
        constexpr EvaluationType getEvaluationType() const { return evalType; }
        constexpr float getLowerThreshold() const { return lowerThreshold; }
        constexpr float getUpperThreshold() const { return upperThreshold; }
        constexpr TimestampMs getConfirmationTimeMs() const { return confirmationTimeMs; }
        constexpr TimestampMs getRecoveryTimeMs() const { return recoveryTimeMs; }
        constexpr TimestampMs getMaximumAgeMs() const { return maximumAgeMs; }
        constexpr SignalError getError() const { return error; }
        constexpr FaultSeverity getSeverity() const { return severity; }
        constexpr FaultType getFaultType() const { return faultType; }
        constexpr FaultLatching getLatching() const { return latching; }

    private:
        SignalId signal;
        EvaluationType evalType;
        float lowerThreshold;
        float upperThreshold;
        TimestampMs confirmationTimeMs;
        TimestampMs recoveryTimeMs;
        TimestampMs maximumAgeMs;
        SignalError error;
        FaultSeverity severity;
        FaultType faultType;
        FaultLatching latching;
        RuleValidationError validateRange() const;
        RuleValidationError validateTimeout() const;
};

#endif
