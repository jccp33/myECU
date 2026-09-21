#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "data_types.hpp"
#include "ecu_state_machine.hpp"
#include "fault_manager.hpp"
#include "message.hpp"
#include <array>
#include <cstddef>

class Control {
    private:
        EcuState state;
    public:
        Control();
        void reset();
        FaultManagerResult processMessages(
            const std::array<Message, MAX_SENSOR_COUNT>& messages,
            std::size_t messageCount,
            FaultManager& faultManager,
            TimestampMs nowMs
        );
        EcuState getCurrentState() const {
            return state;
        }
};

#endif
