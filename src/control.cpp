#include "control.hpp"

Control::Control() : state(EcuState::INIT) {}

void Control::reset() { state = EcuState::INIT; }

FaultManagerResult Control::processMessages(
    const std::array<Message, MAX_SENSOR_COUNT>& messages,
    std::size_t messageCount,
    FaultManager& faultManager,
    TimestampMs nowMs
) {
    if (messageCount > messages.size()) return FaultManagerResult::INVALID_CONFIGURATION;
    bool shutdownRequested = false;
    bool shutdownPermitted = false;
    DiagnosticStatus diagnosticStatus = DiagnosticStatus::AVAILABLE;
    for (std::size_t i = 0U; i < messageCount; ++i) {
        const Message& message = messages[i];
        bool faultConditionActive = false;
        switch (message.getSignalStatus()) {
            case SignalStatus::VALID:
                faultConditionActive = false;
                break;
            case SignalStatus::OUT_OF_RANGE:
            case SignalStatus::TIMEOUT:
                faultConditionActive = true;
                break;
            case SignalStatus::UNDEFINED:
                diagnosticStatus = DiagnosticStatus::EVALUATION_ERROR;
                continue;
        }
        const FaultManagerResult result = faultManager.processCondition(
            message.getSignalId(), 
            faultConditionActive, 
            nowMs
        );
        if (result != FaultManagerResult::OK) {
            diagnosticStatus = toDiagnosticStatus(result);
            const EcuStateInputs inputs(
                faultManager.getSummary(),
                true,
                SelfTestResult::PASSED,
                shutdownRequested,
                shutdownPermitted,
                diagnosticStatus
            );
            state = updateEcuState(
                state,
                inputs
            );
            return result;
        }
        if (
            message.getIsShutdownRequest() && 
            message.getRawValue() == message.getActiveValue()
        ) {
            shutdownRequested = true;
        }
        if (
            message.getIsShutdownPermission() && 
            message.getRawValue() == message.getActiveValue()
        ) {
            shutdownPermitted = true;
        }
    }
    const EcuStateInputs inputs(
        faultManager.getSummary(),
        true,
        SelfTestResult::PASSED,
        shutdownRequested,
        shutdownPermitted,
        diagnosticStatus
    );
    state = updateEcuState(
        state,
        inputs
    );
    return FaultManagerResult::OK;
}
