#include "include/simulations.hpp"
#include "include/config.hpp"
#include "include/fault_configuration.hpp"
#include "include/fault_manager.hpp"
#include "include/signal_store.hpp"
#include <array>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]){
    const SystemConfig config = getSystemConfig();
    if (!isSystemConfigValid(config)) {
        return EXIT_FAILURE;
    }
    const EvaluationRuleSet ruleSet = getEvaluationRuleSet();
    if (validateEvaluationRuleSet(ruleSet, config)
            != FaultConfigurationError::NONE) {
        return EXIT_FAILURE;
    }
    // objects
    std::array<Message, MAX_SENSOR_COUNT> sensors;
    MessageManager mssgManager;
    Gateway gateway;
    // simulations
    SignalStore signalStore;
    FaultManager faultManager(ruleSet.rules, ruleSet.count);
    Control control;
    if (argc > 1 && std::string(argv[1]) == "-auto") {
        randomSimulation(
            config, sensors, mssgManager, gateway,
            signalStore, faultManager, control
        );
    } else {
        userSimulation(
            config, sensors, mssgManager, gateway,
            signalStore, faultManager, control
        );
    }
    // end
    return 0;
}
