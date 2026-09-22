# Comportamiento actual

Este documento resume el comportamiento implementado. La especificación de
transiciones está en [FSM.md](FSM.md).

## Ciclo de procesamiento

1. `MessageManager` inicializa y actualiza los `Message` derivados de
   `SystemConfig`.
2. `Gateway::validateMessage()` evalúa rango y timeout y establece el
   `SignalStatus` de cada mensaje.
3. `Control::processMessages()` convierte cada `SignalStatus` en una condición
   booleana y la entrega a `FaultManager`.
4. Los registros avanzan por `INACTIVE`, `PENDING`, `CONFIRMED`, `RECOVERING` o
   `LATCHED`.
5. `FaultManager::getSummary()` produce un `FaultSummary`.
6. `Control` entrega entradas de dominio a `EcuStateMachine` y conserva el
   estado global resultante.

`SignalSample` y `SignalStore` no forman parte del pipeline vigente del
simulador. La aplicación STM32 todavía utiliza provisionalmente esas
abstracciones y debe alinearse antes de considerarse equivalente al host.

## Prioridad y recuperación

La política general es fail-safe: fallo de self-test o diagnóstico, fallo
crítico, solicitud de apagado, fallo degradado y operación normal.

- Un fallo recuperable se limpia tras permanecer sano durante `recoveryTimeMs`.
- Un fallo latched requiere `FaultManager::resetForIgnitionCycle()`.
- En `SAFE_STATE`, un latched crítico conduce a `SHUTDOWN`.
- Una solicitud normal conduce a `SHUTDOWN_REQ` y espera permiso.
- `SHUTDOWN` es terminal hasta reiniciar `Control`.

## Estado de la evidencia

El CORE y el simulador compilan con CMake y Make. CMake también compila cada
header público de forma aislada y admite el perfil embedded/freestanding.

La suite funcional existente corresponde a una arquitectura anterior y no
está integrada actualmente en CMake ni Make. Por ello, una compilación correcta
no debe interpretarse todavía como validación funcional completa. Consulta
[TESTING.md](TESTING.md) y el checklist de purga arquitectónica.
