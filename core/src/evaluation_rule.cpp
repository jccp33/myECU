#include "evaluation_rule.hpp"

bool EvaluationRule::isValid() const {
    switch (severity) {
        case FaultSeverity::WARNING:
        case FaultSeverity::DEGRADED:
        case FaultSeverity::CRITICAL:
            break;
        case FaultSeverity::NONE:
            return false;
    }
    switch (latching) {
        case FaultLatching::RECOVERABLE:
        case FaultLatching::LATCHED:
            break;
    }
    return true;
}
