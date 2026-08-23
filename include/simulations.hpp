#ifndef SIMULATIONS_HPP
#define SIMULATIONS_HPP

#include "control.hpp"
#include "getaway.hpp"
#include "fault_manager.hpp"
#include "message.hpp"
#include "mssgmanager.hpp"
#include "signal_store.hpp"

#include <array>
#include <cstddef>

constexpr std::size_t USER_MESSAGE_WIDTH = 50U;
#define SPEED_VALUE     500

void userSimulation(
    const SystemConfig& config,
    std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
    MessageManager &mssgManager,
    Gateway &gateway,
    SignalStore& signalStore,
    FaultManager& faultManager,
    Control &control
);

void randomSimulation(
    const SystemConfig& config,
    std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
    MessageManager &mssgManager,
    Gateway &gateway,
    SignalStore& signalStore,
    FaultManager& faultManager,
    Control &control
);

#endif
