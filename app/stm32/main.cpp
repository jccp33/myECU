#include "adc.hpp"
#include "led.hpp"
#include "time.hpp"
#include "config.hpp"
#include "control.hpp"
#include "diagnostic_status.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include "signal_store.hpp"
#include "signal_acquisition.hpp"
#include <cstdint>

int main()
{
    // init objects
    platform::initLed();
    platform::initTime();
    platform::initAdc();
    const SystemConfig systemConfig = getSystemConfig();
    const EvaluationRuleSet ruleSet = getEvaluationRuleSet();
    const FaultConfigurationError configurationError = validateEvaluationRuleSet(ruleSet, systemConfig);
    FaultManager faultManager(ruleSet.rules, ruleSet.count);
    SignalStore signalStore;
    Control control;
    DiagnosticStatus diagnosticStatus = toDiagnosticStatus(configurationError);
    std::uint32_t previousTime = 0U;
    // main loop
    while (true)
    {
        const std::uint32_t now = platform::millis();
        if ((now - previousTime) >= 100U) {
            previousTime = now;
            // read samples
            app::acquireSignals(signalStore, static_cast<TimestampMs>(now));
            // process faults
            if (configurationError == FaultConfigurationError::NONE) {
                const FaultManagerResult faultResult = faultManager.processCycle(
                    signalStore,
                    static_cast<TimestampMs>(now)
                );
                diagnosticStatus = toDiagnosticStatus(faultResult);
            }
            const FaultSummary faultSummary = faultManager.getSummary();
            const EcuStateInputs inputs(
                faultSummary,
                true,
                SelfTestResult::PASSED,
                false,
                false,
                diagnosticStatus
            );
            control.processInputs(inputs);
            // Instrumentación temporal:
            // cambia PC13 una vez por cada ciclo de 100 ms.
            platform::toggleLed();
        }
    }
    // finish program
    return 0;
}
