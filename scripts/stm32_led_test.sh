#!/usr/bin/env bash

set -e

echo "=== [1/3] Building STM32 LED test ==="

mkdir -p build-stm32

arm-none-eabi-g++ \
  -mcpu=cortex-m3 -mthumb -std=c++11 -O0 -g3 \
  -ffreestanding -fno-exceptions -fno-rtti \
  -fno-threadsafe-statics -ffunction-sections -fdata-sections \
  -nostdlib \
  platform/stm32/startup/startup_stm32f103.cpp \
  platform/stm32/src/runtime.cpp \
  platform/stm32/src/time.cpp \
  app/stm32/led_test.cpp \
  -Iplatform/stm32/include \
  -T platform/stm32/linker/stm32f103c8.ld \
  -Wl,--gc-sections \
  -Wl,-Map=build-stm32/led_test.map \
  -lgcc \
  -o build-stm32/led_test.elf

echo
echo "Build successful."

arm-none-eabi-size build-stm32/led_test.elf

echo
echo "=== [2/3] Creating binary ==="

arm-none-eabi-objcopy \
  -O binary \
  build-stm32/led_test.elf \
  build-stm32/led_test.bin

echo
echo "=== [3/3] Flashing Blue Pill ==="

st-flash --reset write \
  build-stm32/led_test.bin \
  0x08000000

echo
echo "=== LED test flashed successfully ==="