# STM32 Hardware Validation

## Objetivo

Validar que el CORE portable de myECU puede ejecutarse sobre hardware STM32
sin depender de la plataforma Linux, usando una plataforma bare-metal mínima.

La plataforma validada actualmente es:

- MCU: STM32F103C8T6
- Arquitectura: ARM Cortex-M3
- Flash detectada: 64 KiB
- SRAM detectada: 20 KiB
- Programador/debugger: ST-LINK V2
- Toolchain: `arm-none-eabi-g++`
- Host de desarrollo: Ubuntu 24.04
- Analizador lógico: Saleae-compatible FX2, 8 canales
- Software de captura: PulseView / sigrok

## Estructura STM32

```text
myECU/
├── app/
│   └── stm32/
│       └── main.cpp
│
├── platform/
│   └── stm32/
│       ├── include/
│       │   ├── led.hpp
│       │   └── time.hpp
│       ├── src/
│       │   ├── led.cpp
│       │   ├── time.cpp
│       │   └── runtime.cpp
│       ├── startup/
│       │   └── startup_stm32f103.cpp
│       └── linker/
│           └── stm32f103c8.ld
│
├── include/
└── src/