#include "../include/utils.hpp"
#include "../include/control.hpp"
#include <cstddef>
#include <iostream>
#include <string>

Control::Control() : state(EcuState::INIT), errors(0) {}

void Control::reset() {
    state = EcuState::INIT;
    errors = 0;
    validSignals.assign(validSignals.size(), false);
}

bool Control::allSignalsValid() {
    for (std::size_t signal = 0; signal < validSignals.size(); signal++) {
        if (!validSignals[signal]) {
            return false;
        }
    }
    return true;
}

bool Control::isWarningCondition(Message &mssg) {
    bool condition = mssg.getRawValue() < mssg.getMinValue() || mssg.getRawValue() > mssg.getMaxValue(); 
    return (condition && !mssg.getIsCritic());
}

bool Control::isCriticalCondition(Message &mssg){
    bool condition = mssg.getRawValue() < mssg.getMinValue() || mssg.getRawValue() > mssg.getMaxValue(); 
    return (condition && mssg.getIsCritic());
}

void Control::processMessage(Message &mssg, uint8_t signal) {
    if (signal < validSignals.size()) {
        validSignals[signal] = mssg.getSignalStatus() == SignalStatus::VALID; 
        validSignals[signal] = validSignals[signal] && !isWarningCondition(mssg); 
        validSignals[signal] = validSignals[signal] && !isCriticalCondition(mssg);
    }

    // SHUT_REQ 
    if (mssg.getSensorId() == SensorId::SHUT_REQ && mssg.getRawValue() == 1.0f) {
        state = EcuState::SHUTDOWN;
        std::cout << TXT_YELLOW << "[CONTROL TRANSITION] SHUTDOWN -> SHUT_REQ" << TXT_RESET << std::endl;
        return;
    }
    // FSM
    switch (state) {
        case EcuState::INIT:
            errors = 0;
            state = EcuState::SELF_TEST;
            break;
        case EcuState::SELF_TEST:
            // voltaje
            if (mssg.getSensorId() == SensorId::VOLTAGE) {
                if (mssg.getRawValue() >= mssg.getMaxValue() && mssg.getSignalStatus() == SignalStatus::VALID) {
                    state = EcuState::OPERATIONAL;
                    std::cout << TXT_GREEN << "[CONTROL] SELF_TEST Exitoso -> OPERATIONAL" << TXT_RESET << std::endl;
                } else {
                    state = EcuState::SAFE_STATE;
                    std::cout << TXT_RED << "[CONTROL] SELF_TEST Fallido -> SAFE_STATE" << TXT_RESET << std::endl;
                }
            }
            break;
        case EcuState::OPERATIONAL:
            // falla critica
            if (isCriticalCondition(mssg)) {
                errors++;
                state = EcuState::SAFE_STATE;
                std::cout << TXT_RED << "[CONTROL] Condición Crítica -> SAFE_STATE" << TXT_RESET << std::endl;
            } 
            // warning
            else if (isWarningCondition(mssg) || mssg.getSignalStatus() != SignalStatus::VALID) {
                errors++;
                state = EcuState::DEGRADED;
                std::cout << TXT_YELLOW << "[CONTROL] Advertencia / Señal no crítica inválida -> DEGRADED" << TXT_RESET << std::endl;
            }
            break;
        case EcuState::DEGRADED:
            // condicion critica en degradado
            if (isCriticalCondition(mssg)) {
                state = EcuState::SAFE_STATE;
                std::cout << TXT_RED << "[CONTROL] Falla Crítica en Modo Degradado -> SAFE_STATE" << TXT_RESET << std::endl;
            }
            // Recuperacion cuando todas las senales vuelven a ser validas
            else if (allSignalsValid()) {
                state = EcuState::OPERATIONAL;
                std::cout << TXT_GREEN << "[CONTROL] Señales Normalizadas -> OPERATIONAL" << TXT_RESET << std::endl;
            }
            break;
        case EcuState::SAFE_STATE:
            // SHUTDOWN 
            if (mssg.getSensorId() == SensorId::SPEED && mssg.getRawValue() == 0.0f) {
                state = EcuState::SHUTDOWN;
                std::cout << TXT_BLUE << "[CONTROL] Vehículo Detenido en Estado Seguro -> SHUTDOWN" << TXT_RESET << std::endl;
            } else if (allSignalsValid()) {
                state = EcuState::OPERATIONAL;
                std::cout << TXT_GREEN << "[CONTROL] Señales Normalizadas -> OPERATIONAL" << TXT_RESET << std::endl;
            }
            break;
        case EcuState::SHUTDOWN:
            // Estado de parada final
            break;
    }
}

void Control::printCurrentState() {
    std::string stateName = "UNDEFINED";
    std::string stateColor = TXT_RESET;
    switch (state) {
        case EcuState::INIT:
            stateName = "INIT";
            stateColor = TXT_BLUE;
            break;
        case EcuState::SELF_TEST:
            stateName = "SELF_TEST";
            stateColor = TXT_YELLOW;
            break;
        case EcuState::OPERATIONAL: 
            stateName = "OPERATIONAL";
            stateColor = TXT_GREEN;
            break;
        case EcuState::DEGRADED:
            stateName = "DEGRADED"; 
            stateColor = TXT_YELLOW;
            break;
        case EcuState::SAFE_STATE:
            stateName = "SAFE_STATE";
            stateColor = TXT_RED;
            break;
        case EcuState::SHUTDOWN:
            stateName = "SHUTDOWN";
            stateColor = TXT_BLUE;
            break;
    }
    std::cout << stateColor << "[CONTROL STATE]: " << stateName << TXT_RESET << std::endl;
}
