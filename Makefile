# Makefile para proyecto ECU

# Compilador
CXX = g++

# Flags de compilación
CXXFLAGS = -Wall -Wextra -pedantic \
           -Wshadow \
           -Wconversion \
           -Wsign-conversion \
           -Wold-style-cast \
           -Wnull-dereference \
           -Wdouble-promotion \
           -std=c++11
CPPFLAGS = -I./include
SANITIZER_FLAGS = -fsanitize=address,undefined \
                  -fno-omit-frame-pointer \
                  -g

# Directorios
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# Archivos fuente
SOURCES = main.cpp \
		  $(SRC_DIR)/message.cpp \
		  $(SRC_DIR)/config.cpp \
		  $(SRC_DIR)/utils.cpp \
		  $(SRC_DIR)/mssgmanager.cpp \
		  $(SRC_DIR)/getaway.cpp \
		  $(SRC_DIR)/control.cpp \
		  $(SRC_DIR)/simulations.cpp \
		  $(SRC_DIR)/linux_platform.cpp

# Archivos objeto
CORE_OBJECTS = $(BUILD_DIR)/message.o \
               $(BUILD_DIR)/config.o \
               $(BUILD_DIR)/mssgmanager.o \
               $(BUILD_DIR)/getaway.o \
               $(BUILD_DIR)/control.o \
			   $(BUILD_DIR)/evaluation_rule.o \
			   $(BUILD_DIR)/fault_state_machine.o \
			   $(BUILD_DIR)/fault_condition_evaluator.o \
			   $(BUILD_DIR)/fault_manager.o \
			   $(BUILD_DIR)/ecu_state_machine.o \
			   $(BUILD_DIR)/signal_store.o \
			   $(BUILD_DIR)/fault_configuration.o \
			   $(BUILD_DIR)/diagnostic_status.o

SIMULATOR_OBJECTS = $(BUILD_DIR)/main.o \
                    $(BUILD_DIR)/utils.o \
                    $(BUILD_DIR)/simulations.o \
                    $(BUILD_DIR)/sensor_simulation.o \
                    $(BUILD_DIR)/linux_platform.o

# Nombre del ejecutable en la raíz del proyecto
TARGET = ecu
CORE_LIBRARY = $(BUILD_DIR)/libecu_core.a
GATEWAY_TEST_TARGET = $(BUILD_DIR)/gateway_tests
GATEWAY_TEST_OBJECT = $(BUILD_DIR)/gateway_tests.o
CONTROL_TEST_TARGET = $(BUILD_DIR)/control_tests
CONTROL_TEST_OBJECT = $(BUILD_DIR)/control_tests.o
FAULT_TYPES_TEST_TARGET = $(BUILD_DIR)/fault_types_tests
FAULT_TYPES_TEST_OBJECT = $(BUILD_DIR)/fault_types_tests.o
EVALUATION_RULE_TEST_TARGET = $(BUILD_DIR)/evaluation_rule_tests
EVALUATION_RULE_TEST_OBJECT = $(BUILD_DIR)/evaluation_rule_tests.o
FAULT_STATE_MACHINE_TEST_TARGET = $(BUILD_DIR)/fault_state_machine_tests
FAULT_STATE_MACHINE_TEST_OBJECT = $(BUILD_DIR)/fault_state_machine_tests.o
FAULT_CONDITION_EVALUATOR_TEST_TARGET = $(BUILD_DIR)/fault_condition_evaluator_tests
FAULT_CONDITION_EVALUATOR_TEST_OBJECT = $(BUILD_DIR)/fault_condition_evaluator_tests.o
FAULT_MANAGER_TEST_TARGET = $(BUILD_DIR)/fault_manager_tests
FAULT_MANAGER_TEST_OBJECT = $(BUILD_DIR)/fault_manager_tests.o
ECU_STATE_MACHINE_TEST_TARGET = $(BUILD_DIR)/ecu_state_machine_tests
ECU_STATE_MACHINE_TEST_OBJECT = $(BUILD_DIR)/ecu_state_machine_test.o
SIGNAL_STORE_TEST_TARGET = $(BUILD_DIR)/signal_store_tests
SIGNAL_STORE_TEST_OBJECT = $(BUILD_DIR)/signal_store_tests.o
FAULT_CONFIGURATION_TEST_TARGET = $(BUILD_DIR)/fault_configuration_tests
FAULT_CONFIGURATION_TEST_OBJECT = $(BUILD_DIR)/fault_configuration_tests.o
DIAGNOSTIC_STATUS_TEST_TARGET = $(BUILD_DIR)/diagnostic_status_tests
DIAGNOSTIC_STATUS_TEST_OBJECT = $(BUILD_DIR)/diagnostic_status_tests.o
ECU_INTEGRATION_TEST_TARGET = $(BUILD_DIR)/ecu_integration_tests
ECU_INTEGRATION_TEST_OBJECT = $(BUILD_DIR)/ecu_integration_tests.o
SENSOR_SIMULATION_TEST_TARGET = $(BUILD_DIR)/sensor_simulation_tests
SENSOR_SIMULATION_TEST_OBJECT = $(BUILD_DIR)/sensor_simulation_tests.o

# Targets
.PHONY: all build compile core test clean run sanitize distclean info

# Target por defecto
all: build

# Crear la compilación del proyecto
build: $(TARGET)
	@echo "✓ Build completado: $(TARGET)"

# Compilar el proyecto
compile: $(TARGET)

# Crear el ejecutable en la raíz
$(TARGET): $(SIMULATOR_OBJECTS) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(SIMULATOR_OBJECTS) $(CORE_LIBRARY)
	@echo "✓ Compilación completada: $(TARGET)"

# Compilar archivos objeto
$(BUILD_DIR)/main.o: main.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/utils.o: $(SRC_DIR)/utils.cpp $(INCLUDE_DIR)/utils.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/message.o: $(SRC_DIR)/message.cpp $(INCLUDE_DIR)/message.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/config.o: $(SRC_DIR)/config.cpp $(INCLUDE_DIR)/config.hpp $(INCLUDE_DIR)/mssgmanager.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/mssgmanager.o: $(SRC_DIR)/mssgmanager.cpp $(INCLUDE_DIR)/mssgmanager.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/getaway.o: $(SRC_DIR)/getaway.cpp $(INCLUDE_DIR)/getaway.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/control.o: $(SRC_DIR)/control.cpp $(INCLUDE_DIR)/control.hpp $(INCLUDE_DIR)/message.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/simulations.o: $(SRC_DIR)/simulations.cpp \
                            $(INCLUDE_DIR)/simulations.hpp \
                            $(INCLUDE_DIR)/sensor_simulation.hpp \
                            $(INCLUDE_DIR)/utils.hpp \
                            $(INCLUDE_DIR)/linux_platform.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/sensor_simulation.o: $(SRC_DIR)/sensor_simulation.cpp \
                                  $(INCLUDE_DIR)/sensor_simulation.hpp \
                                  $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/linux_platform.o: $(SRC_DIR)/linux_platform.cpp $(INCLUDE_DIR)/linux_platform.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/evaluation_rule.o: \
    $(SRC_DIR)/evaluation_rule.cpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/fault_types.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_state_machine.o: \
    $(SRC_DIR)/fault_state_machine.cpp \
    $(INCLUDE_DIR)/fault_state_machine.hpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/fault_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_condition_evaluator.o: \
    $(SRC_DIR)/fault_condition_evaluator.cpp \
    $(INCLUDE_DIR)/fault_condition_evaluator.hpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_manager.o: \
    $(SRC_DIR)/fault_manager.cpp \
    $(INCLUDE_DIR)/fault_manager.hpp \
    $(INCLUDE_DIR)/fault_condition_evaluator.hpp \
    $(INCLUDE_DIR)/fault_state_machine.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/ecu_state_machine.o: \
    $(SRC_DIR)/ecu_state_machine.cpp \
    $(INCLUDE_DIR)/ecu_state_machine.hpp \
    $(INCLUDE_DIR)/fault_types.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/signal_store.o: \
    $(SRC_DIR)/signal_store.cpp \
    $(INCLUDE_DIR)/signal_store.hpp \
    $(INCLUDE_DIR)/signal_sample.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_configuration.o: \
    $(SRC_DIR)/fault_configuration.cpp \
    $(INCLUDE_DIR)/fault_configuration.hpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/diagnostic_status.o: \
    $(SRC_DIR)/diagnostic_status.cpp \
    $(INCLUDE_DIR)/diagnostic_status.hpp \
    $(INCLUDE_DIR)/fault_configuration.hpp \
    $(INCLUDE_DIR)/fault_manager.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

# Compilar y enlazar los tests de Gateway solamente contra el Core
$(GATEWAY_TEST_OBJECT): tests/gateway_tests.cpp \
                        $(INCLUDE_DIR)/getaway.hpp \
                        $(INCLUDE_DIR)/message.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(GATEWAY_TEST_TARGET): $(GATEWAY_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(GATEWAY_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(CONTROL_TEST_OBJECT): tests/control_tests.cpp \
                        $(INCLUDE_DIR)/control.hpp \
                        $(INCLUDE_DIR)/message.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(CONTROL_TEST_TARGET): $(CONTROL_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(CONTROL_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(FAULT_TYPES_TEST_OBJECT): tests/fault_types_tests.cpp \
                          $(INCLUDE_DIR)/fault_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(FAULT_TYPES_TEST_TARGET): $(FAULT_TYPES_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(FAULT_TYPES_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(EVALUATION_RULE_TEST_OBJECT): tests/evaluation_rule_tests.cpp \
                               $(INCLUDE_DIR)/evaluation_rule.hpp \
                               $(INCLUDE_DIR)/fault_types.hpp \
                               $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(EVALUATION_RULE_TEST_TARGET): $(EVALUATION_RULE_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(EVALUATION_RULE_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(FAULT_STATE_MACHINE_TEST_OBJECT): tests/fault_state_machine_tests.cpp \
                                   $(INCLUDE_DIR)/fault_state_machine.hpp \
                                   $(INCLUDE_DIR)/evaluation_rule.hpp \
                                   $(INCLUDE_DIR)/fault_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(FAULT_STATE_MACHINE_TEST_TARGET): $(FAULT_STATE_MACHINE_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(FAULT_STATE_MACHINE_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(FAULT_CONDITION_EVALUATOR_TEST_OBJECT): tests/fault_condition_evaluator_tests.cpp \
                                         $(INCLUDE_DIR)/fault_condition_evaluator.hpp \
                                         $(INCLUDE_DIR)/evaluation_rule.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(FAULT_CONDITION_EVALUATOR_TEST_TARGET): $(FAULT_CONDITION_EVALUATOR_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(FAULT_CONDITION_EVALUATOR_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(FAULT_MANAGER_TEST_OBJECT): tests/fault_manager_tests.cpp \
                              $(INCLUDE_DIR)/fault_manager.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(FAULT_MANAGER_TEST_TARGET): $(FAULT_MANAGER_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(FAULT_MANAGER_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(ECU_STATE_MACHINE_TEST_OBJECT): tests/ecu_state_machine_test.cpp \
                                  $(INCLUDE_DIR)/ecu_state_machine.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(ECU_STATE_MACHINE_TEST_TARGET): $(ECU_STATE_MACHINE_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(ECU_STATE_MACHINE_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(SIGNAL_STORE_TEST_OBJECT): tests/signal_store_tests.cpp \
                             $(INCLUDE_DIR)/signal_store.hpp \
                             $(INCLUDE_DIR)/signal_sample.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(SIGNAL_STORE_TEST_TARGET): $(SIGNAL_STORE_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(SIGNAL_STORE_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(FAULT_CONFIGURATION_TEST_OBJECT): tests/fault_configuration_tests.cpp \
                                    $(INCLUDE_DIR)/fault_configuration.hpp \
                                    $(INCLUDE_DIR)/config.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(FAULT_CONFIGURATION_TEST_TARGET): $(FAULT_CONFIGURATION_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(FAULT_CONFIGURATION_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(DIAGNOSTIC_STATUS_TEST_OBJECT): tests/diagnostic_status_tests.cpp \
                                  $(INCLUDE_DIR)/diagnostic_status.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(DIAGNOSTIC_STATUS_TEST_TARGET): $(DIAGNOSTIC_STATUS_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(DIAGNOSTIC_STATUS_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(ECU_INTEGRATION_TEST_OBJECT): tests/ecu_integration_tests.cpp \
                                $(INCLUDE_DIR)/control.hpp \
                                $(INCLUDE_DIR)/fault_manager.hpp \
                                $(INCLUDE_DIR)/signal_store.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(ECU_INTEGRATION_TEST_TARGET): $(ECU_INTEGRATION_TEST_OBJECT) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(ECU_INTEGRATION_TEST_OBJECT) $(CORE_LIBRARY)
	@echo "✓ Tests enlazados: $@"

$(SENSOR_SIMULATION_TEST_OBJECT): tests/sensor_simulation_tests.cpp \
                                  $(INCLUDE_DIR)/sensor_simulation.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(SENSOR_SIMULATION_TEST_TARGET): $(SENSOR_SIMULATION_TEST_OBJECT) $(BUILD_DIR)/sensor_simulation.o
	$(CXX) $(CXXFLAGS) -o $@ $(SENSOR_SIMULATION_TEST_OBJECT) $(BUILD_DIR)/sensor_simulation.o
	@echo "✓ Tests enlazados: $@"

core: $(CORE_LIBRARY)
	@echo "✓ Core compilado: $(CORE_LIBRARY)"

$(CORE_LIBRARY): $(CORE_OBJECTS)
	ar rcs $@ $^

# Compilar y ejecutar los tests
test: $(GATEWAY_TEST_TARGET) $(CONTROL_TEST_TARGET) $(FAULT_TYPES_TEST_TARGET) $(EVALUATION_RULE_TEST_TARGET) $(FAULT_STATE_MACHINE_TEST_TARGET) $(FAULT_CONDITION_EVALUATOR_TEST_TARGET) $(FAULT_MANAGER_TEST_TARGET) $(ECU_STATE_MACHINE_TEST_TARGET) $(SIGNAL_STORE_TEST_TARGET) $(FAULT_CONFIGURATION_TEST_TARGET) $(DIAGNOSTIC_STATUS_TEST_TARGET) $(ECU_INTEGRATION_TEST_TARGET) $(SENSOR_SIMULATION_TEST_TARGET)
	./$(GATEWAY_TEST_TARGET)
	./$(CONTROL_TEST_TARGET)
	./$(FAULT_TYPES_TEST_TARGET)
	./$(EVALUATION_RULE_TEST_TARGET)
	./$(FAULT_STATE_MACHINE_TEST_TARGET)
	./$(FAULT_CONDITION_EVALUATOR_TEST_TARGET)
	./$(FAULT_MANAGER_TEST_TARGET)
	./$(ECU_STATE_MACHINE_TEST_TARGET)
	./$(SIGNAL_STORE_TEST_TARGET)
	./$(FAULT_CONFIGURATION_TEST_TARGET)
	./$(DIAGNOSTIC_STATUS_TEST_TARGET)
	./$(ECU_INTEGRATION_TEST_TARGET)
	./$(SENSOR_SIMULATION_TEST_TARGET)

# Ejecutar el programa sin recompilar
run:
	@test -x ./$(TARGET) || { echo "ERROR: no existe $(TARGET). Primero ejecuta 'make build'"; exit 1; }
	./$(TARGET)

sanitize:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(SANITIZER_FLAGS)" build

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o $(CORE_LIBRARY) $(GATEWAY_TEST_TARGET) $(CONTROL_TEST_TARGET) $(FAULT_TYPES_TEST_TARGET) $(EVALUATION_RULE_TEST_TARGET) $(FAULT_STATE_MACHINE_TEST_TARGET) $(FAULT_CONDITION_EVALUATOR_TEST_TARGET) $(FAULT_MANAGER_TEST_TARGET) $(ECU_STATE_MACHINE_TEST_TARGET) $(SIGNAL_STORE_TEST_TARGET) $(FAULT_CONFIGURATION_TEST_TARGET) $(DIAGNOSTIC_STATUS_TEST_TARGET) $(ECU_INTEGRATION_TEST_TARGET) $(SENSOR_SIMULATION_TEST_TARGET)
	@echo "✓ Archivos limpios"

# Limpiar todo
distclean: clean
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)
	@echo "✓ Directorio build eliminado"

# Mostrar información
info:
	@echo "=== Información del Proyecto ==="
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Include: $(CPPFLAGS)"
	@echo "Archivos fuente: $(SOURCES)"
	@echo "Ejecutable: $(TARGET)"
