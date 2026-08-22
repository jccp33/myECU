#ifndef CONTROL_HPP
#define CONTROL_HPP

#include "../include/message.hpp"
#include <cstddef>
#include <vector>

#define MAX_INVALID_SIGNALS 3

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
        struct SignalRecord {
            uint32_t messageId;
            SignalStatus status;
            bool critical;
        };
        EcuState state;
        uint32_t errors;
        std::vector<SignalRecord> signals;
        bool shutdownRequested;
        std::size_t maxInvalidSignals;
        bool allSignalsValid() const;
        void updateState();
    public:
        // constructors
        explicit Control(std::size_t invalidSignalLimit = 4);
        // methods
        void reset();
        void processMessage(const Message &mssg);
        EcuState getCurrentState() const { return state; }
};

#endif
