# Roadmap

## Completado: prototipo funcional portable

- [x] Tipos genéricos de señal, error y severidad.
- [x] Reglas configurables de rango y timeout.
- [x] FSM de fallo con confirmación, recuperación y latching.
- [x] Almacenamiento fijo para señales y registros.
- [x] `FaultSummary` desacoplado de sensores.
- [x] FSM global conforme a `FSM.png`, incluido `SHUTDOWN_REQ`.
- [x] `Control` sin `switch` por sensor.
- [x] Diagnóstico fail-safe.
- [x] Integración con simulación manual y automática.
- [x] Modelo gradual de señales.
- [x] CMake, Makefile, perfil embebido y pruebas.
- [x] Verificación host con ASan y UBSan.

## Plataforma embebida STM32 — en progreso

- [x] Seleccionar primer MCU: STM32F103C8T6.
- [x] Validar toolchain ARM `arm-none-eabi-g++`.
- [x] Crear separación `app/stm32` y `platform/stm32`.
- [x] Implementar startup bare-metal mínimo.
- [x] Implementar linker script para 64 KiB Flash / 20 KiB SRAM.
- [x] Validar programación SWD con ST-LINK V2.
- [x] Implementar GPIO bare-metal.
- [x] Implementar SysTick y `millis()`.
- [x] Cross-compilar el CORE para Cortex-M3.
- [x] Ejecutar `FaultManager`, `Control` y FSM global en hardware.
- [x] Validar llegada a `OPERATIONAL`.
- [x] Validar transición a `SAFE_STATE` por timeout.
- [x] Implementar perfil de capacidad STM32 de 16 señales.
- [x] Reducir frame estático de `main()` de ~17.5 KiB a ~2.3 KiB.
- [x] Ejecutar ciclo periódico de 100 ms.
- [x] Verificar temporalmente SysTick y ciclo con analizador lógico.
- [ ] Integrar primera señal física.
- [ ] Definir adquisición independiente de CAN/ADC/SENT.
- [ ] Agregar watchdog.
- [ ] Medir WCET del ciclo real.
- [ ] Medir stack high-water mark.
- [ ] Caracterizar jitter.
- [ ] Validar wrap-around y fuente monotónica de tiempo.

## Diagnóstico y seguridad

- [ ] Definir y persistir DTC y freeze-frames.
- [ ] Integrar servicios UDS necesarios.
- [ ] Crear requisitos trazables para transiciones y reglas.
- [ ] Realizar HARA/ASIL para un caso de uso concreto.
- [ ] Definir cobertura y análisis MISRA/AUTOSAR C++.
- [ ] Agregar SIL, PIL y HIL con inyección de fallos.
- [ ] Revisar ISR, atomicidad y datos compartidos.

## Estado del criterio de primera demo en MCU

El mismo `ecu_core` ya:

- compila con `arm-none-eabi-g++`;
- se ejecuta en STM32F103C8T6;
- ejecuta `FaultManager`, `Control` y la FSM global;
- utiliza un período de aplicación de 100 ms;
- tiene mediciones preliminares de Flash y stack;
- tiene validación externa de temporización con analizador lógico.

Todavía falta para cerrar completamente el criterio:

- recibir muestras desde adaptadores de hardware reales;
- reproducir formalmente el conjunto de pruebas host mediante estrategia
  PIL/HIL o equivalente;
- medir WCET y stack máximo en runtime.
