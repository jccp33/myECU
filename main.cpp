#include "simulations.hpp"
#include "config.hpp"
#include "fault_configuration.hpp"
#include "fault_manager.hpp"
#include <array>
#include <cstdlib>
#include <string>

int main(int argc, char* argv[]) {
    const SystemConfig config = getSystemConfig();
    if (!isSystemConfigValid(config)) return EXIT_FAILURE;

    // Derived diagnostic configuration.
    // SystemConfig remains the single authoritative source.
    std::array<EvaluationRule, MAX_SENSOR_COUNT> ruleStorage;
    EvaluationRuleSet ruleSet;

    const FaultConfigurationError configurationResult = buildEvaluationRuleSet(
        config,
        ruleStorage,
        ruleSet
    );

    if (configurationResult != FaultConfigurationError::NONE) return EXIT_FAILURE;
    if (validateEvaluationRuleSet(ruleSet, config) != FaultConfigurationError::NONE) {
        return EXIT_FAILURE;
    }

    // Runtime objects.
    std::array<Message, MAX_SENSOR_COUNT> sensors;
    MessageManager mssgManager;
    Gateway gateway;
    FaultManager faultManager(
        ruleSet.rules,
        ruleSet.count
    );
    Control control;

    // Simulation.
    if (argc>1 && std::string(argv[1])=="-auto") {
        randomSimulation(
            config,
            sensors,
            mssgManager,
            gateway,
            faultManager,
            control
        );
    } else {
        userSimulation(
            config,
            sensors,
            mssgManager,
            gateway,
            faultManager,
            control
        );
    }

    // Finish.
    return 0;
}
