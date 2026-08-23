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

## Próxima etapa: plataforma embebida

- [ ] Elegir MCU, compilador, memoria y periodos de tareas.
- [ ] Separar físicamente `core/`, `platform/` y `simulator/`.
- [ ] Definir adquisición independiente de CAN/ADC/SENT.
- [ ] Implementar scheduler periódico y watchdog.
- [ ] Medir WCET, stack, RAM, ROM y carga de CPU.
- [ ] Validar wrap-around y fuente monotónica de tiempo.

## Diagnóstico y seguridad

- [ ] Definir y persistir DTC y freeze-frames.
- [ ] Integrar servicios UDS necesarios.
- [ ] Crear requisitos trazables para transiciones y reglas.
- [ ] Realizar HARA/ASIL para un caso de uso concreto.
- [ ] Definir cobertura y análisis MISRA/AUTOSAR C++.
- [ ] Agregar SIL, PIL y HIL con inyección de fallos.
- [ ] Revisar ISR, atomicidad y datos compartidos.

## Criterio de primera demo en MCU

El mismo `ecu_core` debe compilar con el cross-compiler, recibir muestras desde
adaptadores de hardware, ejecutar con periodo conocido, reproducir las pruebas
host y tener mediciones documentadas de memoria, stack y WCET.
