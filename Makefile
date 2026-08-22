# Makefile para proyecto ECU

# Compilador
CXX = g++

# Flags de compilación
CXXFLAGS = -Wall -Wextra -pedantic -std=c++11
CPPFLAGS = -I./include

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
		  $(SRC_DIR)/simulations.cpp

# Archivos objeto
OBJECTS = $(BUILD_DIR)/main.o \
		  $(BUILD_DIR)/message.o \
		  $(BUILD_DIR)/config.o \
		  $(BUILD_DIR)/utils.o \
		  $(BUILD_DIR)/mssgmanager.o \
		  $(BUILD_DIR)/getaway.o \
		  $(BUILD_DIR)/control.o \
		  $(BUILD_DIR)/simulations.o

# Nombre del ejecutable en la raíz del proyecto
TARGET = ecu

# Targets
.PHONY: all build compile clean run distclean info

# Target por defecto
all: build

# Crear la compilación del proyecto
build: $(TARGET)
	@echo "✓ Build completado: $(TARGET)"

# Compilar el proyecto
compile: $(TARGET)

# Crear el ejecutable en la raíz
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
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

$(BUILD_DIR)/simulations.o: $(SRC_DIR)/simulations.cpp $(INCLUDE_DIR)/simulations.hpp $(INCLUDE_DIR)/control.hpp
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	@echo "✓ Compilado: $<"

# Ejecutar el programa sin recompilar
run:
	@test -x ./$(TARGET) || { echo "ERROR: no existe $(TARGET). Primero ejecuta 'make build'"; exit 1; }
	./$(TARGET)

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(BUILD_DIR)/*.o
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
