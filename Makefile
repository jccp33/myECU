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
               $(BUILD_DIR)/control.o

SIMULATOR_OBJECTS = $(BUILD_DIR)/main.o \
                    $(BUILD_DIR)/utils.o \
                    $(BUILD_DIR)/simulations.o \
                    $(BUILD_DIR)/linux_platform.o

# Nombre del ejecutable en la raíz del proyecto
TARGET = ecu
CORE_LIBRARY = $(BUILD_DIR)/libecu_core.a
GATEWAY_TEST_TARGET = $(BUILD_DIR)/gateway_tests
GATEWAY_TEST_OBJECT = $(BUILD_DIR)/gateway_tests.o
CONTROL_TEST_TARGET = $(BUILD_DIR)/control_tests
CONTROL_TEST_OBJECT = $(BUILD_DIR)/control_tests.o

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
                            $(INCLUDE_DIR)/utils.hpp \
                            $(INCLUDE_DIR)/linux_platform.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

$(BUILD_DIR)/linux_platform.o: $(SRC_DIR)/linux_platform.cpp $(INCLUDE_DIR)/linux_platform.hpp
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

core: $(CORE_LIBRARY)
	@echo "✓ Core compilado: $(CORE_LIBRARY)"

$(CORE_LIBRARY): $(CORE_OBJECTS)
	ar rcs $@ $^

# Compilar y ejecutar los tests
test: $(GATEWAY_TEST_TARGET) $(CONTROL_TEST_TARGET)
	./$(GATEWAY_TEST_TARGET)
	./$(CONTROL_TEST_TARGET)

# Ejecutar el programa sin recompilar
run:
	@test -x ./$(TARGET) || { echo "ERROR: no existe $(TARGET). Primero ejecuta 'make build'"; exit 1; }
	./$(TARGET)

sanitize:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(SANITIZER_FLAGS)" build

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o $(CORE_LIBRARY) $(GATEWAY_TEST_TARGET) $(CONTROL_TEST_TARGET)
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
