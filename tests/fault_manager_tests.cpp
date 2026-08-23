#include "fault_manager.hpp"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace {

EvaluationRule createRangeRule(
    TimestampMs confirmationTimeMs = 200U,
    float lower = 0.0F,
    float upper = 100.0F,
    FaultSeverity severity = FaultSeverity::CRITICAL,
    FaultLatching latching = FaultLatching::RECOVERABLE,
    const SignalId& signalId = SignalId(1U, 1U, 100U, 0U)
) {
    return EvaluationRule(
        signalId,
        EvaluationType::RANGE,
        lower,
        upper,
        confirmationTimeMs,
        500U,
        0U,
        SignalError::OUT_OF_RANGE,
        severity,
        FaultType::SENSOR,
        latching
    );
}

EvaluationRule createTimeoutRule(
    TimestampMs confirmationTimeMs = 100U,
    const SignalId& signalId = SignalId(1U, 2U, 101U, 0U),
    TimestampMs maximumAgeMs = 500U,
    TimestampMs recoveryTimeMs = 500U
) {
    return EvaluationRule(
        signalId,
        EvaluationType::TIMEOUT,
        0.0F,
        0.0F,
        confirmationTimeMs,
        recoveryTimeMs,
        maximumAgeMs,
        SignalError::TIMEOUT,
        FaultSeverity::DEGRADED,
        FaultType::COMMUNICATION,
        FaultLatching::RECOVERABLE
    );
}

bool expectTrue(const char* testName, bool condition) {
    if (!condition) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

bool testEmptyConfigurationIsValid() {
    const FaultManager manager(nullptr, 0U);
    return expectTrue(
        "empty fault manager configuration is valid",
        manager.isConfigValid() && manager.getRuleCount() == 0U
    );
}

bool testNullRulesWithNonzeroCountIsInvalid() {
    FaultManager manager(nullptr, 1U);
    return expectTrue(
        "null rules with nonzero count is invalid",
        !manager.isConfigValid()
            && manager.processRule(0U, 0.0F, 0U, 0U)
                == FaultManagerResult::INVALID_CONFIGURATION
            && manager.getRecord(0U) == nullptr
    );
}

bool testRuleCountAboveCapacityIsInvalid() {
    const EvaluationRule rule = createRangeRule();
    FaultManager manager(&rule, MAX_EVALUATION_RULE_COUNT + 1U);
    return expectTrue(
        "rule count above fixed capacity is rejected safely",
        !manager.isConfigValid()
            && manager.processRule(0U, 0.0F, 0U, 0U)
                == FaultManagerResult::INVALID_CONFIGURATION
    );
}

bool testInvalidRuleMakesConfigurationInvalid() {
    const EvaluationRule rules[] = {
        createRangeRule(200U, 100.0F, 0.0F)
    };
    const FaultManager manager(rules, 1U);
    return expectTrue(
        "invalid rule makes manager configuration invalid",
        !manager.isConfigValid() && manager.getRecord(0U) == nullptr
    );
}

bool testValidConfigurationExposesConfiguredRecords() {
    const EvaluationRule rules[] = {createRangeRule()};
    const FaultManager manager(rules, 1U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "valid configuration exposes only configured records",
        manager.isConfigValid()
            && manager.getRuleCount() == 1U
            && record != nullptr
            && record->state == FaultState::INACTIVE
            && record->stateEntryTimeMs == 0U
            && manager.getRecord(1U) == nullptr
    );
}

bool testInvalidRuleIndexIsRejected() {
    const EvaluationRule rules[] = {createRangeRule()};
    FaultManager manager(rules, 1U);
    return expectTrue(
        "processing an invalid rule index is rejected",
        manager.processRule(1U, 50.0F, 0U, 1000U)
            == FaultManagerResult::INVALID_RULE_INDEX
    );
}

bool testHealthyConditionKeepsRecordInactive() {
    const EvaluationRule rules[] = {createRangeRule()};
    FaultManager manager(rules, 1U);
    const FaultManagerResult result = manager.processRule(
        0U, 50.0F, 0U, 1000U
    );
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "healthy condition keeps record inactive",
        result == FaultManagerResult::OK
            && record != nullptr
            && record->state == FaultState::INACTIVE
            && record->stateEntryTimeMs == 0U
    );
}

bool testPersistentConditionBecomesConfirmed() {
    const EvaluationRule rules[] = {createRangeRule(200U)};
    FaultManager manager(rules, 1U);

    const FaultManagerResult pendingResult = manager.processRule(
        0U, 101.0F, 0U, 1000U
    );
    const FaultRecord* pending = manager.getRecord(0U);
    if (!expectTrue(
            "first faulty sample enters pending",
            pendingResult == FaultManagerResult::OK
                && pending != nullptr
                && pending->state == FaultState::PENDING
                && pending->stateEntryTimeMs == 1000U)) {
        return false;
    }

    const FaultManagerResult confirmedResult = manager.processRule(
        0U, 101.0F, 0U, 1200U
    );
    const FaultRecord* confirmed = manager.getRecord(0U);
    return expectTrue(
        "persistent faulty sample becomes confirmed",
        confirmedResult == FaultManagerResult::OK
            && confirmed != nullptr
            && confirmed->state == FaultState::CONFIRMED
            && confirmed->stateEntryTimeMs == 1200U
    );
}

bool testClockErrorDoesNotModifyRecord() {
    const EvaluationRule rules[] = {createTimeoutRule()};
    FaultManager manager(rules, 1U);
    const FaultManagerResult result = manager.processRule(
        0U, 0.0F, 1000U, 999U
    );
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "clock error does not modify fault record",
        result == FaultManagerResult::CLOCK_ERROR
            && record != nullptr
            && record->state == FaultState::INACTIVE
            && record->stateEntryTimeMs == 0U
    );
}

bool testResetClearsConfiguredRecord() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    manager.reset();
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "reset clears configured fault record",
        record != nullptr
            && record->state == FaultState::INACTIVE
            && record->stateEntryTimeMs == 0U
    );
}

bool testManagerResultHasFixedWidthStorage() {
    return expectTrue(
        "FaultManagerResult uses uint8_t storage",
        std::is_same<
            std::underlying_type<FaultManagerResult>::type,
            std::uint8_t
        >::value
    );
}

bool testEmptyConfigurationProducesEmptySummary() {
    const FaultManager manager(nullptr, 0U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "empty configuration produces empty fault summary",
        !summary.hasActiveFaults()
            && summary.activeFaultCount == 0U
            && !summary.hasDegraded
            && !summary.hasCriticalActive
            && !summary.hasCriticalLatched
    );
}

bool testInvalidConfigurationProducesEmptySummary() {
    const FaultManager manager(nullptr, 1U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "invalid configuration produces empty fault summary",
        !summary.hasActiveFaults()
            && summary.activeFaultCount == 0U
            && !summary.hasDegraded
            && !summary.hasCriticalActive
            && !summary.hasCriticalLatched
    );
}

bool testPendingFaultIsNotIncludedInSummary() {
    const EvaluationRule rules[] = {createRangeRule(200U)};
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "pending fault is not included in summary",
        !summary.hasActiveFaults()
            && summary.activeFaultCount == 0U
            && !summary.hasCriticalActive
    );
}

bool testConfirmedDegradedFaultIsIncludedInSummary() {
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::DEGRADED
        )
    };
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "confirmed degraded fault is included in summary",
        summary.hasActiveFaults()
            && summary.activeFaultCount == 1U
            && summary.hasDegraded
            && !summary.hasCriticalActive
            && !summary.hasCriticalLatched
    );
}

bool testRecoveringFaultRemainsIncludedInSummary() {
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::DEGRADED
        )
    };
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    manager.processRule(0U, 50.0F, 0U, 1100U);
    const FaultRecord* record = manager.getRecord(0U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "recovering fault remains included in summary",
        record != nullptr
            && record->state == FaultState::RECOVERING
            && summary.hasActiveFaults()
            && summary.activeFaultCount == 1U
            && summary.hasDegraded
    );
}

bool testConfirmedCriticalFaultIsActiveNotLatched() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "confirmed critical fault is active but not latched",
        summary.activeFaultCount == 1U
            && summary.hasCriticalActive
            && !summary.hasCriticalLatched
    );
}

bool testLatchedCriticalFaultIsReportedSeparately() {
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::CRITICAL,
            FaultLatching::LATCHED
        )
    };
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "latched critical fault is reported separately",
        summary.activeFaultCount == 1U
            && !summary.hasCriticalActive
            && summary.hasCriticalLatched
    );
}

bool testIgnitionCycleResetClearsLatchedFault() {
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::CRITICAL,
            FaultLatching::LATCHED
        )
    };
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);

    const FaultRecord* latchedRecord = manager.getRecord(0U);
    if (!expectTrue(
            "fault is latched before ignition-cycle reset",
            latchedRecord != nullptr
                && latchedRecord->state == FaultState::LATCHED)) {
        return false;
    }

    manager.resetForIgnitionCycle();
    const FaultRecord* resetRecord = manager.getRecord(0U);
    return expectTrue(
        "ignition-cycle reset clears latched fault",
        resetRecord != nullptr
            && resetRecord->state == FaultState::INACTIVE
            && !manager.getSummary().hasActiveFaults()
    );
}

bool testSummaryCountsAllConfirmedSeverities() {
    const EvaluationRule rules[] = {
        createRangeRule(0U, 0.0F, 100.0F, FaultSeverity::WARNING),
        createRangeRule(0U, 0.0F, 100.0F, FaultSeverity::DEGRADED),
        createRangeRule(0U, 0.0F, 100.0F, FaultSeverity::CRITICAL)
    };
    FaultManager manager(rules, 3U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    manager.processRule(1U, 101.0F, 0U, 1000U);
    manager.processRule(2U, 101.0F, 0U, 1000U);
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "summary counts all confirmed severities",
        summary.activeFaultCount == 3U
            && summary.hasDegraded
            && summary.hasCriticalActive
            && !summary.hasCriticalLatched
    );
}

bool testResetClearsFaultSummary() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    manager.processRule(0U, 101.0F, 0U, 1000U);
    manager.reset();
    const FaultSummary summary = manager.getSummary();
    return expectTrue(
        "reset clears fault summary",
        !summary.hasActiveFaults()
            && summary.activeFaultCount == 0U
            && !summary.hasCriticalActive
            && !summary.hasCriticalLatched
            && !summary.hasDegraded
    );
}

bool testProcessSignalRejectsInvalidConfiguration() {
    FaultManager manager(nullptr, 1U);
    const SignalSample sample(
        SignalId(1U, 1U, 100U, 0U),
        50.0F,
        1000U,
        SignalValidity::VALID
    );
    return expectTrue(
        "processSignal rejects invalid manager configuration",
        manager.processSignal(sample, 1000U)
            == FaultManagerResult::INVALID_CONFIGURATION
    );
}

bool testProcessSignalRejectsUnavailableSamples() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    const SignalId id(1U, 1U, 100U, 0U);
    const SignalSample unknown(id, 101.0F, 1000U, SignalValidity::UNKNOWN);
    const SignalSample invalid(id, 101.0F, 1000U, SignalValidity::INVALID);

    const FaultManagerResult unknownResult = manager.processSignal(unknown, 1000U);
    const FaultManagerResult invalidResult = manager.processSignal(invalid, 1000U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "processSignal rejects unavailable samples without modifying records",
        unknownResult == FaultManagerResult::INVALID_SIGNAL_SAMPLE
            && invalidResult == FaultManagerResult::INVALID_SIGNAL_SAMPLE
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testProcessSignalReportsNoMatchingRule() {
    const EvaluationRule rules[] = {createRangeRule()};
    FaultManager manager(rules, 1U);
    const SignalSample sample(
        SignalId(9U, 9U, 999U, 9U),
        50.0F,
        1000U,
        SignalValidity::VALID
    );
    return expectTrue(
        "processSignal reports signal without matching rule",
        manager.processSignal(sample, 1000U)
            == FaultManagerResult::NO_MATCHING_RULE
    );
}

bool testProcessSignalEvaluatesMatchingRangeRule() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    const SignalSample sample(
        SignalId(1U, 1U, 100U, 0U),
        101.0F,
        1000U,
        SignalValidity::VALID
    );
    const FaultManagerResult result = manager.processSignal(sample, 1000U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "processSignal evaluates matching range rule",
        result == FaultManagerResult::OK
            && record != nullptr
            && record->state == FaultState::CONFIRMED
    );
}

bool testProcessSignalUsesCompleteSignalIdentity() {
    const SignalId configuredId(1U, 2U, 100U, 3U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE,
            configuredId
        )
    };
    FaultManager manager(rules, 1U);
    const SignalId mismatches[] = {
        SignalId(9U, 2U, 100U, 3U),
        SignalId(1U, 9U, 100U, 3U),
        SignalId(1U, 2U, 999U, 3U),
        SignalId(1U, 2U, 100U, 9U)
    };

    bool allRejected = true;
    const std::size_t count = sizeof(mismatches) / sizeof(mismatches[0]);
    for (std::size_t index = 0U; index < count; ++index) {
        const SignalSample sample(
            mismatches[index], 101.0F, 1000U, SignalValidity::VALID
        );
        if (manager.processSignal(sample, 1000U)
                != FaultManagerResult::NO_MATCHING_RULE) {
            allRejected = false;
        }
    }

    return expectTrue(
        "processSignal matches all SignalId fields",
        allRejected
    );
}

bool testProcessSignalEvaluatesEveryMatchingRule() {
    const SignalId id(1U, 1U, 100U, 0U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U,
            0.0F,
            100.0F,
            FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE,
            id
        ),
        createTimeoutRule(0U, id)
    };
    FaultManager manager(rules, 2U);
    const SignalSample sample(id, 101.0F, 1000U, SignalValidity::VALID);
    const FaultManagerResult result = manager.processSignal(sample, 1000U);
    const FaultRecord* rangeRecord = manager.getRecord(0U);
    const FaultRecord* timeoutRecord = manager.getRecord(1U);
    return expectTrue(
        "processSignal evaluates every rule for the same signal",
        result == FaultManagerResult::OK
            && rangeRecord != nullptr
            && rangeRecord->state == FaultState::CONFIRMED
            && timeoutRecord != nullptr
            && timeoutRecord->state == FaultState::INACTIVE
    );
}

bool testProcessSignalDoesNotTouchDifferentSignalRules() {
    const SignalId firstId(1U, 1U, 100U, 0U);
    const SignalId secondId(1U, 1U, 200U, 0U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE, firstId
        ),
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::DEGRADED,
            FaultLatching::RECOVERABLE, secondId
        )
    };
    FaultManager manager(rules, 2U);
    manager.processSignal(
        SignalSample(firstId, 101.0F, 1000U, SignalValidity::VALID),
        1000U
    );
    const FaultRecord* first = manager.getRecord(0U);
    const FaultRecord* second = manager.getRecord(1U);
    return expectTrue(
        "processSignal modifies only matching signal records",
        first != nullptr
            && first->state == FaultState::CONFIRMED
            && second != nullptr
            && second->state == FaultState::INACTIVE
    );
}

bool testProcessSignalPropagatesClockError() {
    const SignalId id(1U, 2U, 101U, 0U);
    const EvaluationRule rules[] = {createTimeoutRule(0U, id)};
    FaultManager manager(rules, 1U);
    const SignalSample sample(id, 0.0F, 1000U, SignalValidity::VALID);
    const FaultManagerResult result = manager.processSignal(sample, 999U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "processSignal propagates clock error without modifying record",
        result == FaultManagerResult::CLOCK_ERROR
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testProcessCycleRejectsInvalidConfiguration() {
    FaultManager manager(nullptr, 1U);
    const SignalStore store;
    return expectTrue(
        "processCycle rejects invalid manager configuration",
        manager.processCycle(store, 1000U)
            == FaultManagerResult::INVALID_CONFIGURATION
    );
}

bool testMissingSignalTimeoutUsesMonitoringStart() {
    const EvaluationRule rules[] = {createTimeoutRule(100U)};
    FaultManager manager(rules, 1U);
    const SignalStore store;

    const FaultManagerResult startResult = manager.processCycle(store, 1000U);
    const FaultManagerResult boundaryResult = manager.processCycle(store, 1500U);
    const FaultManagerResult activeResult = manager.processCycle(store, 1501U);
    const FaultRecord* pending = manager.getRecord(0U);
    if (!expectTrue(
            "missing signal starts pending only after maximum age",
            startResult == FaultManagerResult::OK
                && boundaryResult == FaultManagerResult::OK
                && activeResult == FaultManagerResult::OK
                && pending != nullptr
                && pending->state == FaultState::PENDING
                && pending->stateEntryTimeMs == 1501U)) {
        return false;
    }

    manager.processCycle(store, 1601U);
    const FaultRecord* confirmed = manager.getRecord(0U);
    return expectTrue(
        "missing signal confirms at confirmation boundary",
        confirmed != nullptr
            && confirmed->state == FaultState::CONFIRMED
            && confirmed->stateEntryTimeMs == 1601U
    );
}

bool testStoppedSignalProducesTimeout() {
    const SignalId id(1U, 2U, 101U, 0U);
    const EvaluationRule rules[] = {createTimeoutRule(0U, id)};
    FaultManager manager(rules, 1U);
    SignalStore store;
    store.upsert(SignalSample(id, 10.0F, 1000U, SignalValidity::VALID));

    manager.processCycle(store, 1000U);
    manager.processCycle(store, 1500U);
    const FaultRecord* boundary = manager.getRecord(0U);
    if (!expectTrue(
            "stopped signal remains healthy at timeout boundary",
            boundary != nullptr && boundary->state == FaultState::INACTIVE)) {
        return false;
    }

    manager.processCycle(store, 1501U);
    const FaultRecord* timedOut = manager.getRecord(0U);
    return expectTrue(
        "stopped signal times out after maximum age",
        timedOut != nullptr && timedOut->state == FaultState::CONFIRMED
    );
}

bool testTimeoutRecoversAfterFreshSample() {
    const SignalId id(1U, 2U, 101U, 0U);
    const EvaluationRule rules[] = {
        createTimeoutRule(0U, id, 500U, 500U)
    };
    FaultManager manager(rules, 1U);
    SignalStore store;
    manager.processCycle(store, 1000U);
    manager.processCycle(store, 1501U);

    store.upsert(SignalSample(id, 20.0F, 1600U, SignalValidity::VALID));
    manager.processCycle(store, 1600U);
    const FaultRecord* recovering = manager.getRecord(0U);
    if (!expectTrue(
            "fresh sample moves timeout fault to recovering",
            recovering != nullptr
                && recovering->state == FaultState::RECOVERING
                && recovering->stateEntryTimeMs == 1600U)) {
        return false;
    }

    manager.processCycle(store, 2100U);
    const FaultRecord* recovered = manager.getRecord(0U);
    return expectTrue(
        "timeout fault clears at recovery boundary",
        recovered != nullptr && recovered->state == FaultState::INACTIVE
    );
}

bool testProcessCycleEvaluatesValidRangeSample() {
    const SignalId id(1U, 1U, 100U, 0U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE, id
        )
    };
    FaultManager manager(rules, 1U);
    SignalStore store;
    store.upsert(SignalSample(id, 101.0F, 1000U, SignalValidity::VALID));
    const FaultManagerResult result = manager.processCycle(store, 1000U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "processCycle evaluates valid range sample",
        result == FaultManagerResult::OK
            && record != nullptr
            && record->state == FaultState::CONFIRMED
    );
}

bool testMissingRangeSampleDoesNotModifyRecord() {
    const EvaluationRule rules[] = {createRangeRule(0U)};
    FaultManager manager(rules, 1U);
    const SignalStore store;
    const FaultManagerResult result = manager.processCycle(store, 1000U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "missing range sample leaves record unchanged",
        result == FaultManagerResult::OK
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testInvalidRangeSampleIsReportedWithoutEvaluation() {
    const SignalId id(1U, 1U, 100U, 0U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE, id
        )
    };
    FaultManager manager(rules, 1U);
    SignalStore store;
    store.upsert(SignalSample(id, 101.0F, 1000U, SignalValidity::INVALID));
    const FaultManagerResult result = manager.processCycle(store, 1000U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "invalid range sample is reported without evaluation",
        result == FaultManagerResult::INVALID_SIGNAL_SAMPLE
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testInvalidSampleStillRefreshesTimeout() {
    const SignalId id(1U, 2U, 101U, 0U);
    const EvaluationRule rules[] = {createTimeoutRule(0U, id)};
    FaultManager manager(rules, 1U);
    SignalStore store;
    store.upsert(SignalSample(id, 0.0F, 1000U, SignalValidity::INVALID));
    manager.processCycle(store, 1000U);
    const FaultManagerResult result = manager.processCycle(store, 1500U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "invalid received sample still refreshes communication timeout",
        result == FaultManagerResult::OK
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testFutureTimestampPreventsPartialUpdate() {
    const SignalId firstId(1U, 1U, 100U, 0U);
    const SignalId futureId(1U, 1U, 200U, 0U);
    const EvaluationRule rules[] = {
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::CRITICAL,
            FaultLatching::RECOVERABLE, firstId
        ),
        createRangeRule(
            0U, 0.0F, 100.0F, FaultSeverity::DEGRADED,
            FaultLatching::RECOVERABLE, futureId
        )
    };
    FaultManager manager(rules, 2U);
    SignalStore store;
    store.upsert(SignalSample(firstId, 101.0F, 1000U, SignalValidity::VALID));
    store.upsert(SignalSample(futureId, 101.0F, 2000U, SignalValidity::VALID));

    const FaultManagerResult result = manager.processCycle(store, 1000U);
    const FaultRecord* first = manager.getRecord(0U);
    const FaultRecord* second = manager.getRecord(1U);
    return expectTrue(
        "future timestamp prevents every rule update",
        result == FaultManagerResult::CLOCK_ERROR
            && first != nullptr
            && first->state == FaultState::INACTIVE
            && second != nullptr
            && second->state == FaultState::INACTIVE
    );
}

bool testMonitoringClockRegressionIsRejected() {
    const EvaluationRule rules[] = {createTimeoutRule(0U)};
    FaultManager manager(rules, 1U);
    const SignalStore store;
    manager.processCycle(store, 1000U);
    const FaultManagerResult result = manager.processCycle(store, 999U);
    const FaultRecord* record = manager.getRecord(0U);
    return expectTrue(
        "monitoring clock regression is rejected",
        result == FaultManagerResult::CLOCK_ERROR
            && record != nullptr
            && record->state == FaultState::INACTIVE
    );
}

bool testResetStartsNewMonitoringWindow() {
    const EvaluationRule rules[] = {createTimeoutRule(0U)};
    FaultManager manager(rules, 1U);
    const SignalStore store;
    manager.processCycle(store, 1000U);
    manager.processCycle(store, 1501U);
    manager.reset();

    manager.processCycle(store, 10000U);
    manager.processCycle(store, 10500U);
    const FaultRecord* boundary = manager.getRecord(0U);
    if (!expectTrue(
            "reset gives missing signal a new timeout window",
            boundary != nullptr && boundary->state == FaultState::INACTIVE)) {
        return false;
    }

    manager.processCycle(store, 10501U);
    const FaultRecord* timedOut = manager.getRecord(0U);
    return expectTrue(
        "new monitoring window eventually detects timeout",
        timedOut != nullptr && timedOut->state == FaultState::CONFIRMED
    );
}

}  // namespace

int main() {
    int failures = 0;
    const bool results[] = {
        testEmptyConfigurationIsValid(),
        testNullRulesWithNonzeroCountIsInvalid(),
        testRuleCountAboveCapacityIsInvalid(),
        testInvalidRuleMakesConfigurationInvalid(),
        testValidConfigurationExposesConfiguredRecords(),
        testInvalidRuleIndexIsRejected(),
        testHealthyConditionKeepsRecordInactive(),
        testPersistentConditionBecomesConfirmed(),
        testClockErrorDoesNotModifyRecord(),
        testResetClearsConfiguredRecord(),
        testManagerResultHasFixedWidthStorage(),
        testEmptyConfigurationProducesEmptySummary(),
        testInvalidConfigurationProducesEmptySummary(),
        testPendingFaultIsNotIncludedInSummary(),
        testConfirmedDegradedFaultIsIncludedInSummary(),
        testRecoveringFaultRemainsIncludedInSummary(),
        testConfirmedCriticalFaultIsActiveNotLatched(),
        testLatchedCriticalFaultIsReportedSeparately(),
        testIgnitionCycleResetClearsLatchedFault(),
        testSummaryCountsAllConfirmedSeverities(),
        testResetClearsFaultSummary(),
        testProcessSignalRejectsInvalidConfiguration(),
        testProcessSignalRejectsUnavailableSamples(),
        testProcessSignalReportsNoMatchingRule(),
        testProcessSignalEvaluatesMatchingRangeRule(),
        testProcessSignalUsesCompleteSignalIdentity(),
        testProcessSignalEvaluatesEveryMatchingRule(),
        testProcessSignalDoesNotTouchDifferentSignalRules(),
        testProcessSignalPropagatesClockError(),
        testProcessCycleRejectsInvalidConfiguration(),
        testMissingSignalTimeoutUsesMonitoringStart(),
        testStoppedSignalProducesTimeout(),
        testTimeoutRecoversAfterFreshSample(),
        testProcessCycleEvaluatesValidRangeSample(),
        testMissingRangeSampleDoesNotModifyRecord(),
        testInvalidRangeSampleIsReportedWithoutEvaluation(),
        testInvalidSampleStillRefreshesTimeout(),
        testFutureTimestampPreventsPartialUpdate(),
        testMonitoringClockRegressionIsRejected(),
        testResetStartsNewMonitoringWindow()
    };

    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " fault manager test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All fault manager tests passed\n";
    return EXIT_SUCCESS;
}
