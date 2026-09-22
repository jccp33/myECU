#include "evaluation_rule.hpp"
#include <cstdlib>
#include <iostream>

namespace {
bool expectTrue(const char* name, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}
} // namespace

int main() {
    const SignalId id(1U, 2U, 100U, 3U);
    const EvaluationRule valid(id, 200U, 500U, FaultSeverity::CRITICAL, FaultLatching::LATCHED);
    const EvaluationRule noSeverity(id, 200U, 500U, FaultSeverity::NONE, FaultLatching::RECOVERABLE);
    const bool passed =
        expectTrue("rule preserves derived policy",
            valid.getSignalId() == id
            && valid.getConfirmationTimeMs() == 200U
            && valid.getRecoveryTimeMs() == 500U
            && valid.getSeverity() == FaultSeverity::CRITICAL
            && valid.getLatching() == FaultLatching::LATCHED)
        && expectTrue("configured rule is valid", valid.isValid())
        && expectTrue("NONE severity is rejected", !noSeverity.isValid());
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
