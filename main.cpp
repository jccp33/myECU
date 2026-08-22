#include "include/simulations.hpp"
#include "include/config.hpp"
#include <string>
#include <vector>

int main(int argc, char* argv[]){
    const SystemConfig config = getSystemConfig();
    // objects
    std::vector<Message> sensors;
    MessageManager mssgManager;
    Gateway gateway;
    // simulations
    Control control(config.maxInvalidSignals);
    if (argc > 1 && std::string(argv[1]) == "-auto") {
        randomSimulation(config.sensors, sensors, mssgManager, gateway, control);
    } else {
        userSimulation(config.sensors, sensors, mssgManager, gateway, control);
    }
    // end
    return 0;
}

