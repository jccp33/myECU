#ifndef SIMULATIONS_HPP
#define SIMULATIONS_HPP

#include "../include/control.hpp"
#include "../include/getaway.hpp"
#include "../include/message.hpp"
#include "../include/mssgmanager.hpp"
#include <cstddef>

void userSimulation(InitValues INIT_VALUES[], size_t sensors, Message SensorsArray[], MessageManager &mssgManager, Gateway &gateway, Control &control);
void randomSimulation(InitValues INIT_VALUES[], size_t sensors, Message SensorsArray[], MessageManager &mssgManager, Gateway &gateway, Control &control);

#endif
