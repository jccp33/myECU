#include "../include/fault_state_machine.hpp"

#include <cstdlib>
#include <iostream>

namespace {

EvaluationRule createRule(
    TimestampMs confirmationTimeMs,
    TimestampMs recoveryTimeMs,
    FaultLatching latching
) {
    return EvaluationRule(
        SignalId(1U, 1U, 100U, 0U),
        EvaluationType::RANGE,
        0.0F,
        7000.0F,
        confirmationTimeMs,
        recoveryTimeMs,
        0U,
        SignalError::OUT_OF_RANGE,
        FaultSeverity::CRITICAL,
        FaultType::SENSOR,
        latching
    );
}

EvaluationRule createRule(
    TimestampMs confirmationTimeMs,
    FaultLatching latching
) {
    return createRule(confirmationTimeMs, 500U, latching);
}

bool expectRecord(
    const char* testName,
    const FaultRecord& actual,
    FaultState expectedState,
    TimestampMs expectedEntryTime
) {
    if (
        actual.state != expectedState
        || actual.stateEntryTimeMs != expectedEntryTime
    ) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testInactiveHealthyRemainsInactive() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current;
    const FaultRecord next = updateFaultRecord(rule, current, false, 1000U);

    return expectRecord(
        "healthy inactive fault remains inactive",
        next,
        FaultState::INACTIVE,
        0U
    );
}

bool testActiveConditionEntersPending() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord next = updateFaultRecord(rule, FaultRecord(), true, 1000U);

    return expectRecord(
        "active condition enters pending",
        next,
        FaultState::PENDING,
        1000U
    );
}

bool testTransientConditionReturnsInactive() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::PENDING, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1100U);

    return expectRecord(
        "transient condition returns pending fault to inactive",
        next,
        FaultState::INACTIVE,
        1100U
    );
}

bool testPendingBeforeConfirmationKeepsOriginalTime() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::PENDING, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 1199U);

    return expectRecord(
        "pending fault keeps original time before confirmation",
        next,
        FaultState::PENDING,
        1000U
    );
}

bool testExactConfirmationBoundaryConfirms() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::PENDING, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 1200U);

    return expectRecord(
        "exact confirmation boundary confirms recoverable fault",
        next,
        FaultState::CONFIRMED,
        1200U
    );
}

bool testLatchedRuleEntersLatchedState() {
    const EvaluationRule rule = createRule(200U, FaultLatching::LATCHED);
    const FaultRecord current(FaultState::PENDING, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 1200U);

    return expectRecord(
        "confirmed latched rule enters latched state",
        next,
        FaultState::LATCHED,
        1200U
    );
}

bool testZeroConfirmationConfirmsImmediately() {
    const EvaluationRule rule = createRule(0U, FaultLatching::RECOVERABLE);
    const FaultRecord next = updateFaultRecord(rule, FaultRecord(), true, 1000U);

    return expectRecord(
        "zero confirmation time confirms immediately",
        next,
        FaultState::CONFIRMED,
        1000U
    );
}

bool testZeroConfirmationLatchesImmediately() {
    const EvaluationRule rule = createRule(0U, FaultLatching::LATCHED);
    const FaultRecord next = updateFaultRecord(rule, FaultRecord(), true, 1000U);

    return expectRecord(
        "zero confirmation time latches immediately",
        next,
        FaultState::LATCHED,
        1000U
    );
}

bool testClockRegressionRestartsPendingWindow() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::PENDING, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 900U);

    return expectRecord(
        "clock regression restarts pending window",
        next,
        FaultState::PENDING,
        900U
    );
}

bool testConfirmedActiveConditionRemainsConfirmed() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::CONFIRMED, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 1200U);

    return expectRecord(
        "active confirmed fault remains confirmed",
        next,
        FaultState::CONFIRMED,
        1000U
    );
}

bool testConfirmedHealthyConditionStartsRecovery() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::CONFIRMED, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1200U);

    return expectRecord(
        "healthy confirmed fault starts recovery",
        next,
        FaultState::RECOVERING,
        1200U
    );
}

bool testRecoveringBeforeBoundaryKeepsOriginalTime() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::RECOVERING, 1200U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1699U);

    return expectRecord(
        "recovering fault keeps original time before boundary",
        next,
        FaultState::RECOVERING,
        1200U
    );
}

bool testExactRecoveryBoundaryReturnsInactive() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::RECOVERING, 1200U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1700U);

    return expectRecord(
        "exact recovery boundary returns fault to inactive",
        next,
        FaultState::INACTIVE,
        1700U
    );
}

bool testFaultReappearanceCancelsRecovery() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::RECOVERING, 1200U);
    const FaultRecord next = updateFaultRecord(rule, current, true, 1400U);

    return expectRecord(
        "fault reappearance cancels recovery",
        next,
        FaultState::CONFIRMED,
        1400U
    );
}

bool testZeroRecoveryTimeReturnsInactiveImmediately() {
    const EvaluationRule rule = createRule(
        200U,
        0U,
        FaultLatching::RECOVERABLE
    );
    const FaultRecord current(FaultState::CONFIRMED, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1200U);

    return expectRecord(
        "zero recovery time returns fault to inactive immediately",
        next,
        FaultState::INACTIVE,
        1200U
    );
}

bool testClockRegressionRestartsRecoveryWindow() {
    const EvaluationRule rule = createRule(200U, FaultLatching::RECOVERABLE);
    const FaultRecord current(FaultState::RECOVERING, 1200U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 1100U);

    return expectRecord(
        "clock regression restarts recovery window",
        next,
        FaultState::RECOVERING,
        1100U
    );
}

bool testLatchedFaultDoesNotRecover() {
    const EvaluationRule rule = createRule(200U, FaultLatching::LATCHED);
    const FaultRecord current(FaultState::LATCHED, 1000U);
    const FaultRecord next = updateFaultRecord(rule, current, false, 5000U);

    return expectRecord(
        "latched fault remains latched when condition becomes healthy",
        next,
        FaultState::LATCHED,
        1000U
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testInactiveHealthyRemainsInactive(),
        testActiveConditionEntersPending(),
        testTransientConditionReturnsInactive(),
        testPendingBeforeConfirmationKeepsOriginalTime(),
        testExactConfirmationBoundaryConfirms(),
        testLatchedRuleEntersLatchedState(),
        testZeroConfirmationConfirmsImmediately(),
        testZeroConfirmationLatchesImmediately(),
        testClockRegressionRestartsPendingWindow(),
        testConfirmedActiveConditionRemainsConfirmed(),
        testConfirmedHealthyConditionStartsRecovery(),
        testRecoveringBeforeBoundaryKeepsOriginalTime(),
        testExactRecoveryBoundaryReturnsInactive(),
        testFaultReappearanceCancelsRecovery(),
        testZeroRecoveryTimeReturnsInactiveImmediately(),
        testClockRegressionRestartsRecoveryWindow(),
        testLatchedFaultDoesNotRecover()
    };

    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " fault state machine test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All fault state machine tests passed\n";
    return EXIT_SUCCESS;
}
