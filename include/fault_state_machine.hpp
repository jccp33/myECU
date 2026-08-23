#ifndef FAULT_STATE_MACHINE_HPP
#define FAULT_STATE_MACHINE_HPP

#include "data_types.hpp"
#include "evaluation_rule.hpp"
#include "fault_types.hpp"

FaultRecord updateFaultRecord(
    const EvaluationRule &rule,
    const FaultRecord &current,
    bool faultConditionActive,
    TimestampMs nowMs
);

#endif
