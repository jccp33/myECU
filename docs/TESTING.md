## Validación STM32 actual

Ya se validó en STM32F103C8T6:

- startup y vector table;
- linker script;
- programación SWD;
- GPIO bare-metal;
- SysTick de 1 ms;
- ejecución del CORE ARM;
- transición a `OPERATIONAL`;
- transición a `SAFE_STATE`;
- ciclo de aplicación de aproximadamente 100 ms;
- medición temporal externa con analizador lógico;
- reducción de capacidad para adecuar el uso de SRAM.

Resultados detallados:

[STM32 Hardware Validation](STM32_VALIDATION.md)

## Pendiente de validación en hardware

- stack high-water mark en runtime;
- WCET del ciclo completo;
- distribución de jitter;
- wrap-around del timer;
- ISR periféricas y datos compartidos;
- ADC/CAN/SENT reales;
- watchdog;
- PIL/HIL;
- inyección de fallos eléctricos.
