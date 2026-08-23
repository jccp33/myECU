#include "../include/control.hpp"

Control::Control() : state(EcuState::INIT) {}

void Control::reset() {
    state = EcuState::INIT;
}

void Control::processInputs(const EcuStateInputs& inputs) {
    state = updateEcuState(state, inputs);
}
