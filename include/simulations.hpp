#ifndef SIMULATIONS_HPP
#define SIMULATIONS_HPP

#include "../include/control.hpp"
#include "../include/getaway.hpp"
#include "../include/message.hpp"
#include "../include/mssgmanager.hpp"
#include <vector>

#define SPEED_VALUE     500
#define TOLERANCE_VALUE 10.0f
#define USER_MSSG_SIZE  50

struct KeyPressed {
    bool pressed;
    char key;
};

void userSimulation(
    const std::vector<InitValues>& initValues,
    std::vector<Message>& sensorsArray,
    MessageManager &mssgManager,
    Gateway &gateway,
    Control &control
);

void randomSimulation(
    const std::vector<InitValues>& initValues,
    std::vector<Message>& sensorsArray,
    MessageManager &mssgManager,
    Gateway &gateway,
    Control &control
);

#endif
