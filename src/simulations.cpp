#include "../include/simulations.hpp"
#include "../include/utils.hpp"
#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

#define SPEED_VALUE 500

void configureTerminal(bool enable);
bool detectKey(char &tecla);

void initializeMessages(InitValues INIT_VALUES[], size_t sensors, Message SensorsArray[], MessageManager &mssgManager) {
	for (size_t sensor = 0; sensor < sensors; sensor++) {
		mssgManager.InitMessage(INIT_VALUES[sensor], SensorsArray[sensor]);
	}
}

void validateMessages(size_t sensors, Message SensorsArray[], Gateway &gateway) {
	for (size_t sensor = 0; sensor < sensors; sensor++) {
		gateway.validateMessage(SensorsArray[sensor], get_timestamp_ms());
	}
}

void processMessages(size_t sensors, Message SensorsArray[], Control &control) {
	for (size_t sensor = 0; sensor < sensors; sensor++) {
		control.processMessage(SensorsArray[sensor], (uint8_t)sensor);
	}
	control.printCurrentState();
}

void printMessages(size_t sensors, Message SensorsArray[]) {
	for (size_t sensor = 0; sensor < sensors; sensor++) {
		std::cout << SensorsArray[sensor].getStdColorsMessageString() << std::endl;
		//std::cout << SensorsArray[sensor].getMessageString() << std::endl;
	}
}

void userSimulation(
	InitValues INIT_VALUES[], 
	size_t sensors, 
	Message SensorsArray[], 
	MessageManager &mssgManager, 
	Gateway &gateway, 
	Control &control
) {
	// init sensors
	initializeMessages(INIT_VALUES, sensors, SensorsArray, mssgManager);
	// main loop
    while (true) {
		// clean screen
		cleanScreen();
		// show menu
		int option;
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
			for(size_t mssg=0; mssg<sensors; mssg++){
				std::string value_str;
				float value;
				if(SensorsArray[mssg].getSensorId() == SensorId::SHUT_REQ)
					std::cout << "Solicitud de Apagado (0 o 1)?:  ";
				else if(SensorsArray[mssg].getSensorId() == SensorId::BRAKE)
					std::cout << "Solicitud de Freno (0 o 1)?:    ";
				else if(SensorsArray[mssg].getSensorId() == SensorId::SPEED)
					std::cout << "Introduce la velocidad (km/h):  ";
				else if(SensorsArray[mssg].getSensorId() == SensorId::RPM)
					std::cout << "Introduce valor de las RPM:     ";
				else if(SensorsArray[mssg].getSensorId() == SensorId::TEMP)
					std::cout << "Introduce la temperatura (C):   ";
				else if(SensorsArray[mssg].getSensorId() == SensorId::VOLTAGE)
					std::cout << "Introduce valor de voltaje (V): ";
				else 
					std::cout << "Introduce valor de indefinido:  ";
				std::cin >> value_str;
				if(isNumber(value_str)) value = std::stof(value_str);
				else value = 0.0f;
				mssgManager.UpdateMessage(get_timestamp_ms(), value, SensorsArray[mssg]);
			}
			// validate & process
			validateMessages(sensors, SensorsArray, gateway);
			processMessages(sensors, SensorsArray, control);
			//printMessages(sensors, SensorsArray);
		} else if (option == 2) {
			// option 2 : show state
			std::cout << std::endl;
            control.printCurrentState();
            std::cout << std::endl;
			printMessages(sensors, SensorsArray);
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

void randomSimulation(
	InitValues INIT_VALUES[], 
	size_t sensors, 
	Message SensorsArray[], 
	MessageManager &mssgManager, 
	Gateway &gateway, 
	Control &control
) {
	// init sensors
	initializeMessages(INIT_VALUES, sensors, SensorsArray, mssgManager);
	bool isBraked = false;
	bool shutdownRequested = false;
	char command = 0;
	configureTerminal(true);
	// main loop
	while (true) {
		// clean screen
		cleanScreen();
		std::cout << "Para simular freno presione:   'B' o 'b'" << std::endl;
        std::cout << "Para simular apagado presione: 'S' o 's'" << std::endl;
		std::cout << std::endl;
		// update values
		if (detectKey(command) && (command == 'b' || command == 'B')) {
			isBraked = !isBraked;
		} else if (command == 's' || command == 'S') {
			shutdownRequested = true;
		}
		// update values
		for (size_t sensor = 1; sensor < sensors; sensor++) {
			if(SensorsArray[sensor].getSensorId()!=SensorId::BRAKE && SensorsArray[sensor].getSensorId()!=SensorId::SHUT_REQ){
				float min = SensorsArray[sensor].getMinValue() - 20.0f;
				float max = SensorsArray[sensor].getMaxValue() + 20.0f;
				float val = randomFloat(min, max);
				mssgManager.UpdateMessage(get_timestamp_ms(), val, SensorsArray[sensor]);
			}
			if(SensorsArray[sensor].getSensorId() == SensorId::BRAKE){
				mssgManager.UpdateMessage(get_timestamp_ms(), isBraked ? 1.0f : 0.0f, SensorsArray[sensor]);
			}
			if(SensorsArray[sensor].getSensorId() == SensorId::SHUT_REQ){
				mssgManager.UpdateMessage(get_timestamp_ms(), shutdownRequested ? 1.0f : 0.0f, SensorsArray[sensor]);
			}
		}
		// validate sensors
		validateMessages(sensors, SensorsArray, gateway);
		processMessages(sensors, SensorsArray, control);
        std::cout << std::endl;
		printMessages(sensors, SensorsArray);
		// if shutdown request
		if (command == 's' || command == 'S') {
			cleanScreen();
			control.printCurrentState();
			std::cout << std::endl;
			printMessages(sensors, SensorsArray);
			configureTerminal(false);
            std::cout << std::endl;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SPEED_VALUE));
	}
}
