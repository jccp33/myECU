#!/usr/bin/env bash

set -euo pipefail

# ============================================================
# Paths
# ============================================================

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build-stm32"

cd "${PROJECT_DIR}"

# ============================================================
# Check required tools
# ============================================================

command -v arm-none-eabi-g++ >/dev/null 2>&1 || {
    echo "ERROR: arm-none-eabi-g++ is not installed or not in PATH" >&2
    exit 1
}

command -v arm-none-eabi-objcopy >/dev/null 2>&1 || {
    echo "ERROR: arm-none-eabi-objcopy is not installed or not in PATH" >&2
    exit 1
}

command -v arm-none-eabi-size >/dev/null 2>&1 || {
    echo "ERROR: arm-none-eabi-size is not installed or not in PATH" >&2
    exit 1
}

command -v st-flash >/dev/null 2>&1 || {
    echo "ERROR: st-flash is not installed or not in PATH" >&2
    exit 1
}

# ============================================================
# Build directory
# ============================================================

mkdir -p "${BUILD_DIR}"

# ============================================================
# Build
# ============================================================

echo "=== [1/3] Building STM32 RPM input test ==="

arm-none-eabi-g++ \
    -mcpu=cortex-m3 \
    -mthumb \
    -std=c++11 \
    -Og \
    -g3 \
    -ffreestanding \
    -fno-exceptions \
    -fno-rtti \
    -fno-threadsafe-statics \
    -ffunction-sections \
    -fdata-sections \
    -nostdlib \
    -Iplatform/stm32/include \
    -Iapp/stm32 \
    platform/stm32/startup/startup_stm32f103.cpp \
    platform/stm32/src/time.cpp \
    platform/stm32/src/rpm_input.cpp \
    platform/stm32/src/runtime.cpp \
    app/stm32/rpm_sensor.cpp \
    app/stm32/rpm_input_test.cpp \
    -T platform/stm32/linker/stm32f103c8.ld \
    -Wl,--gc-sections \
    -Wl,-Map="${BUILD_DIR}/rpm_input_test.map" \
    -lgcc \
    -o "${BUILD_DIR}/rpm_input_test.elf"

echo
echo "Build successful."
echo

arm-none-eabi-size "${BUILD_DIR}/rpm_input_test.elf"

# ============================================================
# Create binary
# ============================================================

echo
echo "=== [2/3] Creating binary ==="

arm-none-eabi-objcopy \
    -O binary \
    "${BUILD_DIR}/rpm_input_test.elf" \
    "${BUILD_DIR}/rpm_input_test.bin"

echo
echo "Binary created successfully."

# ============================================================
# Flash
# ============================================================

echo
echo "=== [3/3] Flashing Blue Pill ==="

st-flash --reset write \
    "${BUILD_DIR}/rpm_input_test.bin" \
    0x08000000

echo
echo "=== STM32 RPM input test flashed successfully ==="