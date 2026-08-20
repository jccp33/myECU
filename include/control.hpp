#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "../include/message.hpp"

enum class EcuState {
    INIT = 0,
    SELF_TEST = 1,
    OPERATIONAL = 2,
    DEGRADED = 3,
    SAFE_STATE = 4,
    SHUTDOWN = 5
};

class Control {
    private:
        EcuState state;
        uint32_t errors;
        bool validSignals[6];
        bool isWarningCondition(Message& mssg);
        bool isCriticalCondition(Message& mssg);
        bool allSignalsValid();
    public:
        // constructors
        Control();
        // methods
        void reset();
        void processMessage(Message& mssg);
        EcuState getCurrentState(){ return state; }
        void printCurrentState();
};

#endif
