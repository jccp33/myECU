#include "include/message.hpp"
#include "include/mssgmanager.hpp"
#include "include/getaway.hpp"
#include "include/control.hpp"
#include "include/simulations.hpp"
#include "include/config.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string>
#include <vector>

int main(int argc, char* argv[]){
    const SystemConfig config = getSystemConfig();
    // objects
    std::vector<Message> SensorsArray;
    MessageManager mssgManager;
    Gateway gateway;
    // simulations
    Control control(config.maxInvalidSignals);
    if (argc > 1 && std::string(argv[1]) == "-auto") {
        randomSimulation(config.sensors, SensorsArray, mssgManager, gateway, control);
    } else {
        userSimulation(config.sensors, SensorsArray, mssgManager, gateway, control);
    }
    // end
    return 0;
}
