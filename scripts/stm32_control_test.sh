#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-stm32"

cd "${PROJECT_DIR}"

command -v arm-none-eabi-g++ >/dev/null 2>&1 || {
  echo "ERROR: arm-none-eabi-g++ is not installed or not in PATH" >&2
  exit 1
}

command -v arm-none-eabi-objcopy >/dev/null 2>&1 || {
  echo "ERROR: arm-none-eabi-objcopy is not installed or not in PATH" >&2
  exit 1
}

command -v st-flash >/dev/null 2>&1 || {
  echo "ERROR: st-flash is not installed or not in PATH" >&2
  exit 1
}

echo "=== [1/3] Building STM32 myECU control test ==="

mkdir -p "${BUILD_DIR}"

arm-none-eabi-g++ \
  -mcpu=cortex-m3 -mthumb -std=c++11 -Os \
  -ffreestanding \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -ffunction-sections -fdata-sections -nostdlib \
  -DMYECU_MAX_SENSOR_COUNT=16 \
  -Icore/include \
  -Iinclude \
  -Iplatform/stm32/include \
  -Iapp/stm32 \
  platform/stm32/startup/startup_stm32f103.cpp \
  platform/stm32/src/led.cpp \
  platform/stm32/src/time.cpp \
  platform/stm32/src/adc.cpp \
  platform/stm32/src/runtime.cpp \
  src/config.cpp \
  core/src/control.cpp \
  core/src/diagnostic_status.cpp \
  core/src/ecu_state_machine.cpp \
  core/src/evaluation_rule.cpp \
  core/src/fault_configuration.cpp \
  core/src/fault_manager.cpp \
  core/src/fault_state_machine.cpp \
  core/src/getaway.cpp \
  core/src/message.cpp \
  core/src/mssgmanager.cpp \
  app/stm32/signal_acquisition.cpp \
  app/stm32/control_test.cpp \
  -T platform/stm32/linker/stm32f103c8.ld \
  -Wl,--gc-sections \
  -Wl,-Map="${BUILD_DIR}/control_test.map" \
  -lgcc \
  -o "${BUILD_DIR}/control_test.elf"

echo
echo "Build successful."
echo

arm-none-eabi-size "${BUILD_DIR}/control_test.elf"

echo
echo "=== [2/3] Creating binary ==="

arm-none-eabi-objcopy \
  -O binary \
  "${BUILD_DIR}/control_test.elf" \
  "${BUILD_DIR}/control_test.bin"

echo
echo "Binary created successfully."
echo

echo "=== [3/3] Flashing Blue Pill ==="

st-flash --reset write \
  "${BUILD_DIR}/control_test.bin" \
  0x08000000

echo
echo "=== myECU control test flashed successfully ==="
