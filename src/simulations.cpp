#include "../include/simulations.hpp"
#include "../include/utils.hpp"
#include <iostream>
#include <limits>
#include <thread>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <string>
#include <vector>

KeyPressed detectKey(char &tecla) {
    KeyPressed keypressed;
    char c;
    int bytes = read(STDIN_FILENO, &c, 1);
    if (bytes > 0) {
        tecla = c;
        keypressed.key = c;
        keypressed.pressed = true;
        return keypressed;
    }
    keypressed.key = '\0';
    keypressed.pressed = false;
    return keypressed;
}

void configureTerminal(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
}

void initializeMessages(const std::vector<InitValues>& initValues, std::vector<Message>& sensorsArray, MessageManager& mssgManager) {
	for (size_t sensor = 0; sensor < initValues.size(); sensor++) {
		Message mssg = mssgManager.InitMessage(initValues[sensor]);
		sensorsArray.push_back(mssg);
	}
}

void validateMessages(std::vector<Message>& sensorsArray, Gateway& gateway) {
	for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
		gateway.validateMessage(sensorsArray[sensor], get_timestamp_ms());
	}
}

void processMessages(std::vector<Message>& sensorsArray, Control& control) {
	for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
		control.processMessage(sensorsArray[sensor]);
	}
}

void printMessages(const std::vector<Message>& sensorsArray) {
	for (size_t sensor = 0; sensor < sensorsArray.size(); sensor++) {
		std::cout << sensorsArray[sensor].getStdColorsMessageString() << std::endl;
		//std::cout << SensorsArray[sensor].getMessageString() << std::endl;
	}
}

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
				short spaces = USER_MSSG_SIZE - user_mssg.size();
				for(short i=0; i<spaces; i++) user_mssg += " ";
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
            control.printCurrentState();
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
		control.printCurrentState();
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
			control.printCurrentState();
			std::cout << std::endl;
			printMessages(sensorsArray);
			configureTerminal(false);
            std::cout << std::endl;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SPEED_VALUE));
	}
}
