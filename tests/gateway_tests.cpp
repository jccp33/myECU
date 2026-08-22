#include "getaway.hpp"
#include "message.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

Message createMessage(
    float value,
    float minimum,
    float maximum,
    TimestampMs timestamp,
    TimestampMs timeout
) {
    return Message(
        100U,
        SensorId::TPS,
        "Test sensor",
        "V",
        value,
        false,
        minimum,
        maximum,
        timeout,
        false,
        0.0F,
        timestamp
    );
}

bool expectValidatedStatus(
    const std::string& testName,
    float value,
    float minimum,
    float maximum,
    TimestampMs messageTimestamp,
    TimestampMs timeout,
    TimestampMs currentTime,
    SignalStatus expectedStatus
) {
    const Gateway gateway;
    Message message = createMessage(
        value,
        minimum,
        maximum,
        messageTimestamp,
        timeout
    );

    gateway.validateMessage(message, currentTime);

    if (message.getSignalStatus() != expectedStatus) {
        std::cerr << "FAILED: " << testName << '\n';
        return false;
    }

    std::cout << "PASSED: " << testName << '\n';
    return true;
}

}  // namespace

int main() {
    int failures = 0;

    if (!expectValidatedStatus(
            "value inside range",
            2.5F, 0.5F, 4.8F,
            1000U, 500U, 1200U,
            SignalStatus::VALID)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "value below minimum",
            0.49F, 0.5F, 4.8F,
            1000U, 500U, 1200U,
            SignalStatus::OUT_OF_RANGE)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "value above maximum",
            4.81F, 0.5F, 4.8F,
            1000U, 500U, 1200U,
            SignalStatus::OUT_OF_RANGE)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "expired message",
            2.5F, 0.5F, 4.8F,
            1000U, 500U, 1501U,
            SignalStatus::TIMEOUT)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "timeout minus one millisecond",
            2.5F, 0.5F, 4.8F,
            1000U, 500U, 1499U,
            SignalStatus::VALID)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "exactly at timeout",
            2.5F, 0.5F, 4.8F,
            1000U, 500U, 1500U,
            SignalStatus::VALID)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "timeout plus one millisecond",
            2.5F, 0.5F, 4.8F,
            1000U, 500U, 1501U,
            SignalStatus::TIMEOUT)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "value exactly at minimum",
            0.5F, 0.5F, 4.8F,
            1000U, 500U, 1200U,
            SignalStatus::VALID)) {
        ++failures;
    }

    if (!expectValidatedStatus(
            "value exactly at maximum",
            4.8F, 0.5F, 4.8F,
            1000U, 500U, 1200U,
            SignalStatus::VALID)) {
        ++failures;
    }

    if (failures != 0) {
        std::cerr << failures << " Gateway test(s) failed\n";
        return EXIT_FAILURE;
    }

    std::cout << "All Gateway tests passed\n";
    return EXIT_SUCCESS;
}
