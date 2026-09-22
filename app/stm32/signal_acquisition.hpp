#ifndef SIGNAL_ACQUISITION_HPP
#define SIGNAL_ACQUISITION_HPP

#include "message.hpp"
#include "mssgmanager.hpp"
#include <array>
#include <cstddef>

namespace app
{
    void acquireSignals(
        std::array<Message, MAX_SENSOR_COUNT> &messages,
        std::size_t messageCount,
        const MessageManager &messageManager,
        TimestampMs now
    );
}

#endif
