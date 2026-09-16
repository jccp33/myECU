#ifndef SIGNAL_ACQUISITION_HPP
#define SIGNAL_ACQUISITION_HPP

#include "signal_store.hpp"
#include "data_types.hpp"

namespace app
{
    void acquireSignals(
        SignalStore &signalStore,
        TimestampMs now
    );
}

#endif
