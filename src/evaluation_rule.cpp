#include "../include/evaluation_rule.hpp"

RuleValidationError EvaluationRule::validate() const {
    switch (evalType) {
        case EvaluationType::RANGE: 
            return validateRange();
        case EvaluationType::TIMEOUT:
            return validateTimeout();
        case EvaluationType::RATE_OF_CHANGE: 
            return RuleValidationError::INVALID_EVALUATION_TYPE;
    }
    return RuleValidationError::INVALID_EVALUATION_TYPE;
}

RuleValidationError EvaluationRule::validateRange() const {
    if(!(lowerThreshold <= upperThreshold)){
        return RuleValidationError::INVALID_THRESHOLDS;
    }
    if(error != SignalError::OUT_OF_RANGE){
        return RuleValidationError::ERROR_TYPE_MISMATCH;
    }
    if(severity == FaultSeverity::NONE){
        return RuleValidationError::INVALID_SEVERITY;
    }
    return RuleValidationError::NONE;
}

RuleValidationError EvaluationRule::validateTimeout() const {
    if(maximumAgeMs == 0U){
        return RuleValidationError::INVALID_MAXIMUM_AGE;
    }
    if(error != SignalError::TIMEOUT){
        return RuleValidationError::ERROR_TYPE_MISMATCH;
    }
    if(severity == FaultSeverity::NONE){
        return RuleValidationError::INVALID_SEVERITY;
    }
    return RuleValidationError::NONE;
}
