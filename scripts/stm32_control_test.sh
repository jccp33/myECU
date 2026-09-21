#!/usr/bin/env bash

set -e

echo "=== [1/3] Building STM32 myECU control test ==="

mkdir -p build-stm32

arm-none-eabi-g++ \
  -mcpu=cortex-m3 -mthumb -std=c++11 -Os \
  -ffreestanding \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -ffunction-sections -fdata-sections -nostdlib \
  -DMYECU_MAX_SENSOR_COUNT=16 \
  -Iinclude \
  -Iplatform/stm32/include \
  -Iapp/stm32 \
  platform/stm32/startup/startup_stm32f103.cpp \
  platform/stm32/src/led.cpp \
  platform/stm32/src/time.cpp \
  platform/stm32/src/adc.cpp \
  platform/stm32/src/runtime.cpp \
  src/config.cpp \
  src/control.cpp \
  src/diagnostic_status.cpp \
  src/ecu_state_machine.cpp \
  src/evaluation_rule.cpp \
  src/fault_configuration.cpp \
  src/fault_manager.cpp \
  src/fault_state_machine.cpp \
  src/signal_sample.cpp \
  src/signal_store.cpp \
  app/stm32/signal_acquisition.cpp \
  app/stm32/control_test.cpp \
  -T platform/stm32/linker/stm32f103c8.ld \
  -Wl,--gc-sections \
  -Wl,-Map=build-stm32/control_test.map \
  -lgcc \
  -o build-stm32/control_test.elf

echo
echo "Build successful."
echo

arm-none-eabi-size build-stm32/control_test.elf

echo
echo "=== [2/3] Creating binary ==="

arm-none-eabi-objcopy \
  -O binary \
  build-stm32/control_test.elf \
  build-stm32/control_test.bin

echo
echo "Binary created successfully."
echo

echo "=== [3/3] Flashing Blue Pill ==="

st-flash --reset write \
  build-stm32/control_test.bin \
  0x08000000

echo
echo "=== myECU control test flashed successfully ==="