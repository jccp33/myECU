#include "adc.hpp"
#include "time.hpp"
#include "config.hpp"
#include "control.hpp"
#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include "signal_store.hpp"
#include "signal_acquisition.hpp"
#include "led.hpp"
#include <cstdint>

volatile bool g_hasDegraded = false;
volatile bool g_hasCriticalActive = false;
volatile std::uint8_t g_ecuState = 0U;
volatile std::uint8_t g_tpsFaultState = 0U;

namespace {
    void insertNominalSignals(SignalStore &signalStore, TimestampMs now){
        const SignalSample nominalSignals[] = {
            // SignalId                   value   time  validity
            {SignalId(1U, 1U, 100U, 0U),   0.0F,  now, SignalValidity::VALID}, // SHUT_REQ
            {SignalId(1U, 1U, 101U, 0U),   0.0F,  now, SignalValidity::VALID}, // BRAKE
            {SignalId(1U, 1U, 102U, 0U),  50.0F,  now, SignalValidity::VALID}, // SPEED
            {SignalId(1U, 1U, 103U, 0U), 1500.0F, now, SignalValidity::VALID}, // RPM
            {SignalId(1U, 1U, 104U, 0U),  90.0F,  now, SignalValidity::VALID}, // TEMP
            {SignalId(1U, 1U, 105U, 0U),  12.5F,  now, SignalValidity::VALID}, // VOLTAGE
            // 106 TPS NO se simula: provendrá físicamente del ADC.
            {SignalId(1U, 1U, 107U, 0U),   2.5F,  now, SignalValidity::VALID}, // MAP
            {SignalId(1U, 1U, 108U, 0U),  20.0F,  now, SignalValidity::VALID}, // MAF
            {SignalId(1U, 1U, 109U, 0U),   0.5F,  now, SignalValidity::VALID}  // O2
        };
        for(const SignalSample &sample : nominalSignals){
            const SignalStoreResult result = signalStore.upsert(sample);
            (void)result;
        }
    }
} // namespace

int main(){
    // init objects
    platform::initLeds();
    platform::initTime();
    platform::initAdc();
    const SystemConfig systemConfig = getSystemConfig();
    const EvaluationRuleSet ruleSet = getEvaluationRuleSet();
    const FaultConfigurationError configurationError =
        validateEvaluationRuleSet(ruleSet, systemConfig);
    FaultManager faultManager(ruleSet.rules, ruleSet.count);
    SignalStore signalStore;
    Control control;
    DiagnosticStatus diagnosticStatus = toDiagnosticStatus(configurationError);
    std::uint32_t previousTime = 0U;
    // main loop
    while(true){
        const std::uint32_t now = platform::millis();
        if((now - previousTime) >= 100U){
            previousTime = now;
            // mantener senales nominales sin adquisicion fisica
            insertNominalSignals(signalStore, static_cast<TimestampMs>(now));
            // TPS: 106 proviene del ADC fisico
            app::acquireSignals(signalStore, static_cast<TimestampMs>(now));
            if(configurationError == FaultConfigurationError::NONE){
                const FaultManagerResult faultResult = faultManager.processCycle(
                    signalStore,
                    static_cast<TimestampMs>(now)
                );
                diagnosticStatus = toDiagnosticStatus(faultResult);
            }
            const FaultSummary faultSummary = faultManager.getSummary();

            g_hasDegraded = faultSummary.hasDegraded;
            g_hasCriticalActive = faultSummary.hasCriticalActive;
            const FaultRecord *tpsFaultRecord = faultManager.getRecord(12U);
            if (tpsFaultRecord != nullptr){
                g_tpsFaultState = static_cast<std::uint8_t>(tpsFaultRecord->state);
            }

            const EcuStateInputs inputs(
                faultSummary,
                true,
                SelfTestResult::PASSED,
                false,
                false,
                diagnosticStatus
            );
            control.processInputs(inputs);
            g_ecuState = static_cast<std::uint8_t>(control.getCurrentState());
            // use leds to show ECU state
            const EcuState currState = control.getCurrentState();
            platform::turnAllLedsOff();
            switch(currState){
                case EcuState::INIT:
                case EcuState::SELF_TEST:
                    platform::turnLedOn(platform::Led::BLUE);
                    break;
                case EcuState::OPERATIONAL:
                    platform::turnLedOn(platform::Led::GREEN);
                    break;
                case EcuState::DEGRADED:
                    platform::turnLedOn(platform::Led::YELLOW);
                    break;
                case EcuState::SAFE_STATE:
                    platform::turnLedOn(platform::Led::RED);
                    break;
                case EcuState::SHUTDOWN_REQ:
                case EcuState::SHUTDOWN:
                    break;
            }
        }
    }
    // finish
    return 0;
}
