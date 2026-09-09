#include "config.hpp"
#include "fault_manager.hpp"
#include "signal_store.hpp"
#include "control.hpp"
#include "signal_sample.hpp"
#include "fault_types.hpp"

extern "C" {

__attribute__((used))
const unsigned SIZE_SYSTEM_CONFIG = sizeof(SystemConfig);

__attribute__((used))
const unsigned SIZE_SIGNAL_STORE = sizeof(SignalStore);

__attribute__((used))
const unsigned SIZE_FAULT_MANAGER = sizeof(FaultManager);

__attribute__((used))
const unsigned SIZE_CONTROL = sizeof(Control);

__attribute__((used))
const unsigned SIZE_SIGNAL_SAMPLE = sizeof(SignalSample);

__attribute__((used))
const unsigned SIZE_FAULT_RECORD = sizeof(FaultRecord);

}
