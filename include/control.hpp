#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "ecu_state_machine.hpp"

class Control {
    private:
        EcuState state;
    public:
        Control();
        void reset();
        void processInputs(const EcuStateInputs& inputs);
        EcuState getCurrentState() const { return state; }
};

#endif
