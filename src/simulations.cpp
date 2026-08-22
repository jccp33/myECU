#include "../include/simulations.hpp"
#include "../include/utils.hpp"
#include "../include/linux_platform.hpp"
#include <iostream>
#include <thread>
#include <iomanip>
#include <chrono>
#include <limits>
#include <string>

// SIMULATION
void initializeMessages(
	const std::vector<InitValues> &initValues, 
	std::vector<Message> &sensorsArray, 
	MessageManager &mssgManager
) {
	sensorsArray.reserve(initValues.size());
	for (size_t sensor = 0; sensor < initValues.size(); sensor++) {
		sensorsArray.push_back(mssgManager.InitMessage(
			initValues[sensor], 
			get_timestamp_ms()
		));
	}
}

// SIMULATION
void validateMessages(std::vector<Message>& sensorsArray, Gateway& gateway) {
	for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
		gateway.validateMessage(sensorsArray[sensor], get_timestamp_ms());
	}
}

// SIMULATION
void processMessages(std::vector<Message>& sensorsArray, Control& control) {
	for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
		control.processMessage(sensorsArray[sensor]);
	}
}

// PRESENTATION
const char* getSignalStatusText(SignalStatus status) {
    switch (status) {
        case SignalStatus::VALID:
            return "valido";

        case SignalStatus::OUT_OF_RANGE:
            return "fuera de rango";

        case SignalStatus::TIMEOUT:
            return "fuera de tiempo";

        case SignalStatus::UNDEFINED:
            return "indefinido";
    }
    return "indefinido";
}

// PRESENTATION
const char* getSignalStatusColor(const Message& message) {
    switch (message.getSignalStatus()) {
        case SignalStatus::VALID:
            return TXT_GREEN;
        case SignalStatus::OUT_OF_RANGE:
        case SignalStatus::TIMEOUT:
            return message.getIsCritic() ? TXT_RED : TXT_YELLOW;
        case SignalStatus::UNDEFINED:
            return TXT_YELLOW;
    }
    return TXT_YELLOW;
}

// PRESENTATION
void printMessages(const std::vector<Message> &sensorsArray) {
    for (std::size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
        const Message& message = sensorsArray[sensor];
        std::cout
            << std::left
            << std::setw(24) << message.getName()
            << std::setw(10) << std::fixed << std::setprecision(2)
            << message.getRawValue()
            << std::setw(6) << message.getUnit()
            << getSignalStatusColor(message)
            << getSignalStatusText(message.getSignalStatus())
            << TXT_RESET
            << std::endl;
    }
}

// PRESENTATION
void printControlState(const Control &control) {
    const char* stateName = "UNDEFINED";
    const char* stateColor = TXT_RESET;
    switch (control.getCurrentState()) {
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
    std::cout
        << stateColor
        << "[CONTROL STATE]: "
        << stateName
        << TXT_RESET
        << std::endl;
}

// SIMULATION
void userSimulation(
	const std::vector<InitValues>& initValues,
	std::vector<Message>& sensorsArray,
	MessageManager &mssgManager, 
	Gateway &gateway, 
	Control &control
) {
	// init sensors
	initializeMessages(initValues, sensorsArray, mssgManager);
	// main loop
    while (true) {
		// clean screen
		cleanScreen();
		// show menu
		short option;
		std::cout << "1. Ingresar valores de senales" << std::endl;
		std::cout << "2. Mostrar estado del sistema" << std::endl;
		std::cout << "3. Salir" << std::endl;
		std::cout << "Selecciona una opcion: ";
		std::cin >> option;
		// invalid option
		if (option<1 || option>3) {
			std::cout << TXT_RED << "Opcion invalida." << TXT_RESET << std::endl;
			std::cout << std::endl << "Presione enter para continuar ...";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cin.get();
			continue;
		}
		if (option == 1) {
			std::cout << std::endl;
			// option 1 : read signals
			for(size_t mssg=0; mssg<sensorsArray.size(); mssg++){
				std::string value_str;
				float value;
				// user message
				std::string user_mssg = std::string("Introduce valor de ")
					+ sensorsArray[mssg].getName()
					+ " (" + sensorsArray[mssg].getUnit() + "):";
				if (user_mssg.size() < USER_MESSAGE_WIDTH) {
					user_mssg.append(USER_MESSAGE_WIDTH - user_mssg.size(), ' ');
				}
				// show user message and read value
				std::cout << user_mssg;
				std::cin >> value_str;
				if(isNumber(value_str)) value = std::stof(value_str);
				else value = 0.0f;
				mssgManager.UpdateMessage(get_timestamp_ms(), value, sensorsArray[mssg]);
			}
			// validate & process
			validateMessages(sensorsArray, gateway);
			processMessages(sensorsArray, control);
		} else if (option == 2) {
			// option 2 : show state
			std::cout << std::endl;
            printControlState(control);
            std::cout << std::endl;
			printMessages(sensorsArray);
			std::cout << std::endl << "Presione enter para continuar ...";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cin.get();
		} else if (option == 3) {
			// option 3 : finish
			return;
		} else {
			// invalid option
			std::cout << TXT_RED << "Opcion invalida." << TXT_RESET << std::endl;
		}
	}
}

// SIMULATION
void randomSimulation(
	const std::vector<InitValues>& initValues,
	std::vector<Message>& sensorsArray,
	MessageManager &mssgManager, 
	Gateway &gateway, 
	Control &control
) {
	// init sensors
	initializeMessages(initValues, sensorsArray, mssgManager);
	bool isBraked = false;
	bool shutdownRequested = false;
	configureTerminal(true);
	// main loop
	while (true) {
		// clean screen and print states
		cleanScreen();
		std::cout << "Para simular freno presione:   'B' o 'b'" << std::endl;
        std::cout << "Para simular apagado presione: 'S' o 's'" << std::endl;
		std::cout << std::endl;
		printControlState(control);
		std::cout << std::endl;
		printMessages(sensorsArray);
		std::cout << std::endl;
		// update values
		char command = 0;
		KeyPressed keypressed = detectKey(command);
		if (keypressed.pressed && (keypressed.key == 'b' || keypressed.key == 'B')) {
			isBraked = !isBraked;
		} else if (keypressed.pressed && (keypressed.key == 's' || keypressed.key == 'S')) {
			shutdownRequested = true;
		}
		// update values
		for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
			if(sensorsArray[sensor].getSensorId()!=SensorId::BRAKE && sensorsArray[sensor].getSensorId()!=SensorId::SHUT_REQ){
				float min = sensorsArray[sensor].getMinValue() - TOLERANCE_VALUE;
				float max = sensorsArray[sensor].getMaxValue() + TOLERANCE_VALUE;
				float val = randomFloat(min, max);
				mssgManager.UpdateMessage(get_timestamp_ms(), val, sensorsArray[sensor]);
			}
			if(sensorsArray[sensor].getSensorId()==SensorId::BRAKE){
				mssgManager.UpdateMessage(get_timestamp_ms(), isBraked ? 1.0f : 0.0f, sensorsArray[sensor]);
			}
			if(sensorsArray[sensor].getSensorId()==SensorId::SHUT_REQ){
				mssgManager.UpdateMessage(get_timestamp_ms(), shutdownRequested ? 1.0f : 0.0f, sensorsArray[sensor]);
			}
		}
		// validate sensors
		validateMessages(sensorsArray, gateway);
		processMessages(sensorsArray, control);
		// if shutdown request
		if (shutdownRequested) {
			cleanScreen();
			printControlState(control);
			std::cout << std::endl;
			printMessages(sensorsArray);
			configureTerminal(false);
            std::cout << std::endl;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SPEED_VALUE));
	}
}
