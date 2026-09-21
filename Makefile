# Makefile para proyecto myECU
#
# Este Makefile compila:
#   1. La librería estática del ECU Core.
#   2. El simulador Linux.
#
# Los tests están excluidos temporalmente mientras se completa
# la purga y alineación arquitectónica del proyecto.


# ============================================================
# Compilador y flags
# ============================================================

CXX = g++

CXXFLAGS = -Wall \
           -Wextra \
           -pedantic \
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


# ============================================================
# Directorios
# ============================================================

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build


# ============================================================
# Ejecutable y librería
# ============================================================

TARGET = ecu
CORE_LIBRARY = $(BUILD_DIR)/libecu_core.a


# ============================================================
# ECU Core
# ============================================================

CORE_OBJECTS = $(BUILD_DIR)/message.o \
               $(BUILD_DIR)/config.o \
               $(BUILD_DIR)/mssgmanager.o \
               $(BUILD_DIR)/getaway.o \
               $(BUILD_DIR)/control.o \
               $(BUILD_DIR)/evaluation_rule.o \
               $(BUILD_DIR)/fault_state_machine.o \
               $(BUILD_DIR)/fault_manager.o \
               $(BUILD_DIR)/ecu_state_machine.o \
               $(BUILD_DIR)/signal_store.o \
               $(BUILD_DIR)/fault_configuration.o \
               $(BUILD_DIR)/diagnostic_status.o


# ============================================================
# Simulador Linux
# ============================================================

SIMULATOR_OBJECTS = $(BUILD_DIR)/main.o \
                    $(BUILD_DIR)/utils.o \
                    $(BUILD_DIR)/simulations.o \
                    $(BUILD_DIR)/sensor_simulation.o \
                    $(BUILD_DIR)/linux_platform.o


# ============================================================
# Targets principales
# ============================================================

.PHONY: all build compile core clean distclean run sanitize info

all: build

build: $(TARGET)
	@echo "✓ Build completado: $(TARGET)"

compile: $(TARGET)

core: $(CORE_LIBRARY)
	@echo "✓ Core compilado: $(CORE_LIBRARY)"


# ============================================================
# Ejecutable
# ============================================================

$(TARGET): $(SIMULATOR_OBJECTS) $(CORE_LIBRARY)
	$(CXX) $(CXXFLAGS) -o $@ $(SIMULATOR_OBJECTS) $(CORE_LIBRARY)
	@echo "✓ Compilación completada: $(TARGET)"


# ============================================================
# Librería estática ECU Core
# ============================================================

$(CORE_LIBRARY): $(CORE_OBJECTS)
	ar rcs $@ $^
	@echo "✓ Librería creada: $(CORE_LIBRARY)"


# ============================================================
# Objetos del simulador
# ============================================================

$(BUILD_DIR)/main.o: main.cpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/utils.o: \
    $(SRC_DIR)/utils.cpp \
    $(INCLUDE_DIR)/utils.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/simulations.o: \
    $(SRC_DIR)/simulations.cpp \
    $(INCLUDE_DIR)/simulations.hpp \
    $(INCLUDE_DIR)/sensor_simulation.hpp \
    $(INCLUDE_DIR)/utils.hpp \
    $(INCLUDE_DIR)/linux_platform.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/sensor_simulation.o: \
    $(SRC_DIR)/sensor_simulation.cpp \
    $(INCLUDE_DIR)/sensor_simulation.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/linux_platform.o: \
    $(SRC_DIR)/linux_platform.cpp \
    $(INCLUDE_DIR)/linux_platform.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"


# ============================================================
# Objetos ECU Core
# ============================================================

$(BUILD_DIR)/message.o: \
    $(SRC_DIR)/message.cpp \
    $(INCLUDE_DIR)/message.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/config.o: \
    $(SRC_DIR)/config.cpp \
    $(INCLUDE_DIR)/config.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/mssgmanager.o: \
    $(SRC_DIR)/mssgmanager.cpp \
    $(INCLUDE_DIR)/mssgmanager.hpp \
    $(INCLUDE_DIR)/message.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/getaway.o: \
    $(SRC_DIR)/getaway.cpp \
    $(INCLUDE_DIR)/getaway.hpp \
    $(INCLUDE_DIR)/message.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/control.o: \
    $(SRC_DIR)/control.cpp \
    $(INCLUDE_DIR)/control.hpp \
    $(INCLUDE_DIR)/message.hpp \
    $(INCLUDE_DIR)/fault_manager.hpp \
    $(INCLUDE_DIR)/ecu_state_machine.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/evaluation_rule.o: \
    $(SRC_DIR)/evaluation_rule.cpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_state_machine.o: \
    $(SRC_DIR)/fault_state_machine.cpp \
    $(INCLUDE_DIR)/fault_state_machine.hpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/fault_manager.o: \
    $(SRC_DIR)/fault_manager.cpp \
    $(INCLUDE_DIR)/fault_manager.hpp \
    $(INCLUDE_DIR)/fault_state_machine.hpp \
    $(INCLUDE_DIR)/evaluation_rule.hpp \
    $(INCLUDE_DIR)/data_types.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/ecu_state_machine.o: \
    $(SRC_DIR)/ecu_state_machine.cpp \
    $(INCLUDE_DIR)/ecu_state_machine.hpp \
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


# ============================================================
# Ejecución
# ============================================================

run:
	@test -x ./$(TARGET) || { \
		echo "ERROR: no existe $(TARGET). Primero ejecuta 'make build'"; \
		exit 1; \
	}
	./$(TARGET)


# ============================================================
# Sanitizers
# ============================================================

sanitize:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(SANITIZER_FLAGS)" build


# ============================================================
# Limpieza
# ============================================================

clean:
	rm -f $(TARGET)
	rm -f $(BUILD_DIR)/*.o
	rm -f $(CORE_LIBRARY)
	@echo "✓ Archivos de compilación eliminados"

distclean: clean
	rm -rf $(BUILD_DIR)
	@echo "✓ Directorio build eliminado"


# ============================================================
# Información
# ============================================================

info:
	@echo "=== Información del Proyecto ==="
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Include: $(CPPFLAGS)"
	@echo "Core: $(CORE_LIBRARY)"
	@echo "Ejecutable: $(TARGET)"