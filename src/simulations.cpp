#include "../include/simulations.hpp"
#include "../include/utils.hpp"
#include <chrono>
#include <iostream>
#include <limits>
#include <thread>

#define SPEED_VALUE 500

void configureTerminal(bool enable);
bool detectKey(char &tecla);

namespace {
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
		control.processMessage(SensorsArray[sensor], sensors);
	}
	control.printCurrentState();
}

void printMessages(size_t sensors, Message SensorsArray[]) {
	for (size_t sensor = 0; sensor < sensors; sensor++) {
		std::cout << SensorsArray[sensor].getStdColorsMessageString() << std::endl;
	}
}

bool readFloat(const char *prompt, float &value) {
	while (true) {
		std::cout << prompt;
		if (std::cin >> value) {
			return true;
		}
		if (std::cin.eof()) {
			return false;
		}
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << TXT_RED << "Entrada invalida. Introduce un valor numerico." << TXT_RESET << std::endl;
	}
}
}

void userSimulation(InitValues INIT_VALUES[], size_t sensors, Message SensorsArray[], MessageManager &mssgManager, Gateway &gateway, Control &control) {
	initializeMessages(INIT_VALUES, sensors, SensorsArray, mssgManager);
	
    while (true) {
		cleanScreen();
		std::cout << "1. Ingresar valores de senales" << std::endl;
		std::cout << "2. Mostrar estado del sistema" << std::endl;
		std::cout << "3. Reiniciar sistema" << std::endl;
		std::cout << "4. Salir" << std::endl;
		std::cout << std::endl << "Selecciona una opcion: ";

		int option;
		if (!(std::cin >> option)) {
			if (std::cin.eof()) {
				return;
			}
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << TXT_RED << "Opcion invalida." << TXT_RESET << std::endl;
			continue;
		}

		if (option == 1) {
			float speed;
			float rpm;
			float temperature;
			float voltage;
			float brake;
			float shutdown;
			if (!readFloat("Introduce valor velocidad km/h: ", speed) ||
				!readFloat("Introduce valor RPM: ", rpm) ||
				!readFloat("Introduce temperatura C: ", temperature) ||
				!readFloat("Introduce voltaje V: ", voltage) ||
				!readFloat("Introduce valor freno (0 o 1): ", brake) ||
				!readFloat("Introduce shutdown (0 o 1): ", shutdown)) {
				return;
			}
			mssgManager.UpdateMessage(get_timestamp_ms(), speed, SensorsArray[1]);
			mssgManager.UpdateMessage(get_timestamp_ms(), rpm, SensorsArray[2]);
			mssgManager.UpdateMessage(get_timestamp_ms(), temperature, SensorsArray[3]);
			mssgManager.UpdateMessage(get_timestamp_ms(), voltage, SensorsArray[4]);
			mssgManager.UpdateMessage(get_timestamp_ms(), brake, SensorsArray[5]);
			mssgManager.UpdateMessage(get_timestamp_ms(), shutdown, SensorsArray[0]);
			validateMessages(sensors, SensorsArray, gateway);
			processMessages(sensors, SensorsArray, control);
			printMessages(sensors, SensorsArray);
		} else if (option == 2) {
			std::cout << std::endl;
            control.printCurrentState();
            std::cout << std::endl;
			printMessages(sensors, SensorsArray);
			std::cout << std::endl << "Press enter to continue ...";
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cin.get();
		} else if (option == 3) {
			initializeMessages(INIT_VALUES, sensors, SensorsArray, mssgManager);
			control.reset(sensors);
			std::cout << TXT_GREEN << "Sistema reiniciado." << TXT_RESET << std::endl;
		} else if (option == 4) {
			return;
		} else {
			std::cout << TXT_YELLOW << "Opcion invalida." << TXT_RESET << std::endl;
		}
	}
}

void randomSimulation(InitValues INIT_VALUES[], size_t sensors, Message SensorsArray[], MessageManager &mssgManager, Gateway &gateway, Control &control) {
	initializeMessages(INIT_VALUES, sensors, SensorsArray, mssgManager);
	bool isBraked = false;
	bool shutdownRequested = false;
	char command = 0;
	configureTerminal(true);

	while (true) {
		cleanScreen();
		for (size_t sensor = 1; sensor < 5; sensor++) {
			float range = INIT_VALUES[sensor].maxValue - INIT_VALUES[sensor].minValue;
			float margin = range * 0.25f;
			float value = randomFloat(INIT_VALUES[sensor].minValue - margin, INIT_VALUES[sensor].maxValue + margin);
			mssgManager.UpdateMessage(get_timestamp_ms(), value, SensorsArray[sensor]);
		}

		if (detectKey(command) && (command == 'b' || command == 'B')) {
			isBraked = !isBraked;
		} else if (command == 's' || command == 'S') {
			shutdownRequested = true;
		}

		uint64_t currentTime = get_timestamp_ms();
		mssgManager.UpdateMessage(currentTime, isBraked ? 1.0f : 0.0f, SensorsArray[5]);
		mssgManager.UpdateMessage(currentTime, shutdownRequested ? 1.0f : 0.0f, SensorsArray[0]);

		validateMessages(sensors, SensorsArray, gateway);
		processMessages(sensors, SensorsArray, control);
        std::cout << std::endl;
		printMessages(sensors, SensorsArray);

		if (command == 's' || command == 'S') {
			configureTerminal(false);
            std::cout << std::endl;
			return;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(SPEED_VALUE));
	}
}
