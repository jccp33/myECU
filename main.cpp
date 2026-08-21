#include "include/message.hpp"
#include "include/mssgmanager.hpp"
#include "include/getaway.hpp"
#include "include/control.hpp"
#include "include/simulations.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

void configureTerminal(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
}

bool detectKey(char &tecla) {
    char c;
    int bytes = read(STDIN_FILENO, &c, 1);
    if (bytes > 0) {
        tecla = c;
        return true;
    }
    return false;
}

int main(int argc, char* argv[]){
    // init values
    InitValues INIT_VALUES[] = {
        {0, SensorId::SHUT_REQ, 00.0f, 000.0f, 0001.0f, false, "S_R"},
        {1, SensorId::BRAKE,    00.0f, 000.0f, 0001.0f, false, "BRK"},
        {2, SensorId::SPEED,    00.0f, 000.0f, 0220.0f, false, "km/h"},
        {3, SensorId::RPM,      00.0f, 000.0f, 7000.0f, true,  "RPM"},
        {4, SensorId::TEMP,     25.0f, -20.0f, 0130.0f, true,  "C"},
        {5, SensorId::VOLTAGE,  12.5f, 008.0f, 0016.0f, true,  "V"}
    };
    const size_t sensors = sizeof(INIT_VALUES) / sizeof(InitValues);
    // objects
    Message SensorsArray[sensors];
    MessageManager mssgManager;
    Gateway gateway;
    Control control;
    // simulations
    if (argc > 1 && std::string(argv[1]) == "-auto") {
        randomSimulation(INIT_VALUES, sensors, SensorsArray, mssgManager, gateway, control);
    } else {
        userSimulation(INIT_VALUES, sensors, SensorsArray, mssgManager, gateway, control);
    }
    // end
    return 0;
}
