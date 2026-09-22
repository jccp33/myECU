#include "getaway.hpp"
#include "message.hpp"
#include <cstdlib>
#include <iostream>

namespace {
bool expectStatus(const char* name, Message message, TimestampMs now, SignalStatus expected) {
    const Gateway gateway;
    gateway.validateMessage(message, now);
    if (message.getSignalStatus() != expected) {
        std::cerr << "FAILED: " << name << '\n';
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}
bool expectClockError(const char* name, Message message, TimestampMs now) {
    const Gateway gateway;
    const GatewayResult result = gateway.validateMessage(message, now);
    if (result != GatewayResult::CLOCK_ERROR) {
        std::cerr << "FAILED: " << name << " - expected CLOCK_ERROR\n";
        return false;
    }
    if (message.getSignalStatus() != SignalStatus::UNDEFINED) {
        std::cerr << "FAILED: " << name << " - expected UNDEFINED\n";
        return false;
    }
    std::cout << "PASSED: " << name << '\n';
    return true;
}
Message message(float value, float minimum, float maximum, TimestampMs timestamp, TimestampMs timeout) {
    return Message(SignalId(1U, 1U, 100U, 0U), value, minimum, maximum,
        timeout, false, false, 0.0F, timestamp);
}
} // namespace

int main() {
    const bool passed =
        expectStatus("value inside range is valid", message(50.0F, 0.0F, 100.0F, 100U, 50U), 150U, SignalStatus::VALID)
        && expectStatus("minimum is inclusive", message(0.0F, 0.0F, 100.0F, 100U, 50U), 100U, SignalStatus::VALID)
        && expectStatus("maximum is inclusive", message(100.0F, 0.0F, 100.0F, 100U, 50U), 100U, SignalStatus::VALID)
        && expectStatus("value below range is rejected", message(-1.0F, 0.0F, 100.0F, 100U, 50U), 100U, SignalStatus::OUT_OF_RANGE)
        && expectStatus("value above range is rejected", message(101.0F, 0.0F, 100.0F, 100U, 50U), 100U, SignalStatus::OUT_OF_RANGE)
        && expectStatus("timeout boundary is valid", message(50.0F, 0.0F, 100.0F, 100U, 50U), 150U, SignalStatus::VALID)
        && expectStatus("age above timeout expires", message(50.0F, 0.0F, 100.0F, 100U, 50U), 151U, SignalStatus::TIMEOUT)
        && expectStatus("timeout takes precedence over range", message(101.0F, 0.0F, 100.0F, 100U, 50U), 151U, SignalStatus::TIMEOUT)
        && expectClockError("clock regression is detected", message(50.0F, 0.0F, 100.0F, 100U, 50U), 99U);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
