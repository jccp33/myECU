#include "../include/simulations.hpp"
#include "../include/utils.hpp"
#include "../include/linux_platform.hpp"
#include "../include/sensor_simulation.hpp"
#include <iostream>
#include <thread>
#include <iomanip>
#include <chrono>
#include <limits>
#include <string>

// SIMULATION
void initializeMessages(
	const SystemConfig& config,
	std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
	MessageManager &mssgManager
) {
	for (std::size_t sensor = 0; sensor < config.sensorCount; sensor++) {
		sensorsArray[sensor] = mssgManager.InitMessage(
			config.sensors[sensor],
			get_timestamp_ms()
		);
	}
}

// SIMULATION
void validateMessages(
	std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
	std::size_t sensorCount,
	Gateway& gateway
) {
	for (std::size_t sensor = 0; sensor < sensorCount; sensor++) {
		gateway.validateMessage(
			sensorsArray[sensor],
			get_timestamp_ms()
		);
	}
}

// SIMULATION
void processMessages(
	const SystemConfig& config,
	std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
	std::size_t sensorCount,
	SignalStore& signalStore,
	FaultManager& faultManager,
	Control& control
) {
	DiagnosticStatus diagnosticStatus = DiagnosticStatus::AVAILABLE;
	bool shutdownRequested = false;
	bool shutdownPermitted = false;
	const TimestampMs nowMs = get_timestamp_ms();
	for (std::size_t sensor = 0; sensor < sensorCount; sensor++) {
		const Message& message = sensorsArray[sensor];
		const InitValues& metadata = config.sensors[sensor];
		const SignalStoreResult storeResult = signalStore.upsert(
			SignalSample(
				metadata.signalId,
				message.getRawValue(),
				message.getTimestamp(),
				SignalValidity::VALID
			)
		);
		if (storeResult == SignalStoreResult::CAPACITY_EXCEEDED) {
			diagnosticStatus = DiagnosticStatus::EVALUATION_ERROR;
		} else if (storeResult == SignalStoreResult::STALE_SAMPLE) {
			diagnosticStatus = DiagnosticStatus::CLOCK_ERROR;
		}
		if (metadata.isShutdownRequest
				&& message.getRawValue() == metadata.activeValue) {
			shutdownRequested = true;
		}
		if (metadata.sId == SensorId::BRAKE
				&& message.getRawValue() == metadata.activeValue) {
			shutdownPermitted = true;
		}
	}
	const FaultManagerResult cycleResult = faultManager.processCycle(
		signalStore,
		nowMs
	);
	if (diagnosticStatus == DiagnosticStatus::AVAILABLE) {
		diagnosticStatus = toDiagnosticStatus(cycleResult);
	}
	control.processInputs(EcuStateInputs(
		faultManager.getSummary(),
		true,
		SelfTestResult::PASSED,
		shutdownRequested,
		shutdownPermitted,
		diagnosticStatus
	));
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
const InitValues* findSignalMetadata(
    const SystemConfig& config,
    uint32_t messageId
) {
    for (std::size_t index = 0; index < config.sensorCount; ++index) {
        if (config.sensors[index].id == messageId) {
            return &config.sensors[index];
        }
    }
    return 0;
}

// PRESENTATION
void printMessages(
    const SystemConfig& config,
    const std::array<Message, MAX_SENSOR_COUNT>& sensorsArray
) {
    for (std::size_t sensor = 0; sensor < config.sensorCount; sensor++) {
        const Message& message = sensorsArray[sensor];
        const InitValues* metadata = findSignalMetadata(
            config,
            message.getMessageId()
        );
        const char* name = metadata != 0 ? metadata->name : "Desconocida";
        const char* unit = metadata != 0 ? metadata->unit : "";
        std::cout
            << std::left
            << std::setw(24) << name
            << std::setw(10) << std::fixed << std::setprecision(2)
            << message.getRawValue()
            << std::setw(6) << unit
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
        case EcuState::SHUTDOWN_REQ:
            stateName = "SHUTDOWN_REQ";
            stateColor = TXT_BLUE;
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
	const SystemConfig& config,
	std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
	MessageManager &mssgManager, 
	Gateway &gateway, 
	SignalStore& signalStore,
	FaultManager& faultManager,
	Control &control
) {
	if (config.sensorCount > sensorsArray.size()) {
		return;
	}
	// init sensors
	initializeMessages(config, sensorsArray, mssgManager);
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
			for (std::size_t mssg = 0; mssg < config.sensorCount; mssg++) {
				std::string value_str;
				float value;
				const InitValues* metadata = findSignalMetadata(
					config,
					sensorsArray[mssg].getMessageId()
				);
				const char* name = metadata != 0 ? metadata->name : "Desconocida";
				const char* unit = metadata != 0 ? metadata->unit : "";
				// user message
				std::string user_mssg = std::string("Introduce valor de ")
					+ name
					+ " (" + unit + "):";
				if (user_mssg.size() < USER_MESSAGE_WIDTH) {
					user_mssg.append(USER_MESSAGE_WIDTH - user_mssg.size(), ' ');
				}
				// show user message and read value
				std::cout << user_mssg;
				std::cin >> value_str;
				if(isNumber(value_str)) value = std::stof(value_str);
				else value = 0.0f;
				mssgManager.UpdateMessage(
					get_timestamp_ms(),
					value,
					sensorsArray[mssg]
				);
			}
			// validate & process
			validateMessages(sensorsArray, config.sensorCount, gateway);
			processMessages(
				config, sensorsArray, config.sensorCount,
				signalStore, faultManager, control
			);
		} else if (option == 2) {
			// option 2 : show state
			std::cout << std::endl;
            printControlState(control);
            std::cout << std::endl;
			printMessages(config, sensorsArray);
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
	const SystemConfig& config,
	std::array<Message, MAX_SENSOR_COUNT>& sensorsArray,
	MessageManager &mssgManager, 
	Gateway &gateway, 
	SignalStore& signalStore,
	FaultManager& faultManager,
	Control &control
) {
	if (config.sensorCount > sensorsArray.size()) {
		return;
	}
	// init sensors
	initializeMessages(config, sensorsArray, mssgManager);
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
		printMessages(config, sensorsArray);
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
		for (std::size_t sensor = 0; sensor < config.sensorCount; sensor++) {
			if(
				sensorsArray[sensor].getSensorId()!=SensorId::BRAKE &&
				sensorsArray[sensor].getSensorId()!=SensorId::SHUT_REQ
			){
				const float val = simulateSensorValue(
					sensorsArray[sensor].getSensorId(),
					sensorsArray[sensor].getRawValue(),
					randomFloat(-1.0F, 1.0F)
				);
				mssgManager.UpdateMessage(
					get_timestamp_ms(),
					val,
					sensorsArray[sensor]
				);
			}
			if(sensorsArray[sensor].getSensorId()==SensorId::BRAKE){
				mssgManager.UpdateMessage(
					get_timestamp_ms(),
					isBraked ? 1.0f : 0.0f,
					sensorsArray[sensor]
				);
			}
			if(sensorsArray[sensor].getSensorId()==SensorId::SHUT_REQ){
				mssgManager.UpdateMessage(
					get_timestamp_ms(),
					shutdownRequested ? 1.0f : 0.0f,
					sensorsArray[sensor]
				);
			}
		}
		// validate sensors
		validateMessages(sensorsArray, config.sensorCount, gateway);
		processMessages(
			config, sensorsArray, config.sensorCount,
			signalStore, faultManager, control
		);
		// if shutdown request
		if (control.getCurrentState() == EcuState::SHUTDOWN) {
			cleanScreen();
			printControlState(control);
			std::cout << std::endl;
			printMessages(config, sensorsArray);
			configureTerminal(false);
            std::cout << std::endl;
			return;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(SPEED_VALUE));
	}
}
