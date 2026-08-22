#ifndef SIMULATIONS_HPP
#define SIMULATIONS_HPP

#include "../include/control.hpp"
#include "../include/getaway.hpp"
#include "../include/message.hpp"
#include "../include/mssgmanager.hpp"
#include <cstddef>
#include <vector>

constexpr std::size_t USER_MESSAGE_WIDTH = 50U;
#define SPEED_VALUE     500
#define TOLERANCE_VALUE 10.0f

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
