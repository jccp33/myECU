#include "control.hpp"
#include "message.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

Message createMessage(
    uint32_t messageId,
    SignalStatus status,
    bool critical = false,
    bool shutdownRequest = false,
    float value = 0.0F,
    float activeValue = 1.0F,
    SensorId sensorId = SensorId::TPS
) {
    Message message(
        messageId,
        sensorId,
        "Test sensor",
        "V",
        value,
        critical,
        0.0F,
        5.0F,
        500U,
        shutdownRequest,
        activeValue,
        1000U
    );
    message.setSignalStatus(status);
    return message;
}

bool expectState(
    const std::string& testName,
    const Control& control,
    EcuState expected
) {
    if (control.getCurrentState() != expected) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

void initializeOperational(Control& control) {
    control.processMessage(createMessage(100U, SignalStatus::VALID));
    control.processMessage(createMessage(100U, SignalStatus::VALID));
}

bool testInitialState() {
    const Control control;
    return expectState("initial state is INIT", control, EcuState::INIT);
}

bool testInitToSelfTest() {
    Control control;
    control.processMessage(createMessage(100U, SignalStatus::VALID));
    return expectState("INIT to SELF_TEST", control, EcuState::SELF_TEST);
}

bool testSelfTestToOperational() {
    Control control;
    initializeOperational(control);
    return expectState(
        "SELF_TEST to OPERATIONAL",
        control,
        EcuState::OPERATIONAL
    );
}

bool testOperationalToDegraded() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE));
    return expectState(
        "OPERATIONAL to DEGRADED on non-critical failure",
        control,
        EcuState::DEGRADED
    );
}

bool testDegradedToOperational() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::TIMEOUT));
    control.processMessage(createMessage(101U, SignalStatus::VALID));
    return expectState(
        "DEGRADED to OPERATIONAL after recovery",
        control,
        EcuState::OPERATIONAL
    );
}

bool testOperationalToSafeState() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE, true));
    return expectState(
        "OPERATIONAL to SAFE_STATE on critical failure",
        control,
        EcuState::SAFE_STATE
    );
}

bool testSafeStateToOperational() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::TIMEOUT, true));
    control.processMessage(createMessage(101U, SignalStatus::VALID, true));
    return expectState(
        "SAFE_STATE to OPERATIONAL after recovery",
        control,
        EcuState::OPERATIONAL
    );
}

bool testSafeStateToShutdown() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE, true));
    control.processMessage(
        createMessage(102U, SignalStatus::VALID, false, true, 1.0F, 1.0F)
    );
    return expectState(
        "SAFE_STATE to SHUTDOWN on active request",
        control,
        EcuState::SHUTDOWN
    );
}

bool testMultipleInvalidSignals() {
    Control control(2U);
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE));
    control.processMessage(createMessage(102U, SignalStatus::TIMEOUT));
    if (!expectState(
            "invalid signals at limit remain DEGRADED",
            control,
            EcuState::DEGRADED)) {
        return false;
    }

    control.processMessage(createMessage(103U, SignalStatus::OUT_OF_RANGE));
    return expectState(
        "invalid signals above limit enter SAFE_STATE",
        control,
        EcuState::SAFE_STATE
    );
}

bool testCriticalFailureThenRecovery() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE, true));
    control.processMessage(createMessage(101U, SignalStatus::VALID, true));
    return expectState(
        "critical failure followed by recovery",
        control,
        EcuState::OPERATIONAL
    );
}

bool testNonCriticalThenCriticalFailure() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE));
    if (!expectState(
            "non-critical failure first enters DEGRADED",
            control,
            EcuState::DEGRADED)) {
        return false;
    }

    control.processMessage(createMessage(102U, SignalStatus::TIMEOUT, true));
    return expectState(
        "critical failure after non-critical enters SAFE_STATE",
        control,
        EcuState::SAFE_STATE
    );
}

bool testInvalidSensorId() {
    Control control;
    initializeOperational(control);
    const SensorId invalidSensorId = static_cast<SensorId>(255U);
    control.processMessage(
        createMessage(
            101U,
            SignalStatus::OUT_OF_RANGE,
            false,
            false,
            0.0F,
            1.0F,
            invalidSensorId
        )
    );
    return expectState(
        "invalid SensorId is processed by message metadata",
        control,
        EcuState::DEGRADED
    );
}

bool testRepeatedMessageDoesNotDuplicateSignal() {
    Control control(1U);
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE));
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE));
    return expectState(
        "repeated message updates instead of duplicating signal",
        control,
        EcuState::DEGRADED
    );
}

bool testShutdownIsTerminal() {
    Control control;
    initializeOperational(control);
    control.processMessage(createMessage(101U, SignalStatus::OUT_OF_RANGE, true));
    control.processMessage(
        createMessage(102U, SignalStatus::VALID, false, true, 1.0F, 1.0F)
    );
    control.processMessage(createMessage(101U, SignalStatus::VALID, true));
    control.processMessage(
        createMessage(102U, SignalStatus::VALID, false, true, 0.0F, 1.0F)
    );
    return expectState(
        "SHUTDOWN remains terminal after later messages",
        control,
        EcuState::SHUTDOWN
    );
}

bool testReset() {
    Control control;
    initializeOperational(control);
    control.reset();
    return expectState("reset returns to INIT", control, EcuState::INIT);
}

}  // namespace

int main() {
    int failures = 0;

    const bool results[] = {
        testInitialState(),
        testInitToSelfTest(),
        testSelfTestToOperational(),
        testOperationalToDegraded(),
        testDegradedToOperational(),
        testOperationalToSafeState(),
        testSafeStateToOperational(),
        testSafeStateToShutdown(),
        testMultipleInvalidSignals(),
        testCriticalFailureThenRecovery(),
        testNonCriticalThenCriticalFailure(),
        testInvalidSensorId(),
        testRepeatedMessageDoesNotDuplicateSignal(),
        testShutdownIsTerminal(),
        testReset()
    };

    const std::size_t resultCount = sizeof(results) / sizeof(results[0]);
    for (std::size_t result = 0; result < resultCount; ++result) {
        if (!results[result]) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << failures << " Control test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All Control tests passed\n";
    return EXIT_SUCCESS;
}
