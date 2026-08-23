# Comportamiento actual

Este documento resume el comportamiento implementado. La especificación de
transiciones está en [FSM.md](FSM.md).

## Ciclo de procesamiento

1. La aplicación convierte cada `Message` en un `SignalSample`.
2. `SignalStore::upsert()` conserva la muestra más reciente por `SignalId`.
3. `FaultManager::processCycle()` evalúa las reglas configuradas.
4. Los registros avanzan por `INACTIVE`, `PENDING`, `CONFIRMED`, `RECOVERING` o
   `LATCHED`.
5. `FaultManager::getSummary()` produce un `FaultSummary`.
6. `Control::processInputs()` actualiza la FSM de la ECU.

## Prioridad y recuperación

La política general es fail-safe: fallo de self-test o diagnóstico, fallo
crítico, solicitud de apagado, fallo degradado y operación normal.

- Un fallo recuperable se limpia tras permanecer sano durante `recoveryTimeMs`.
- Un fallo latched requiere `FaultManager::resetForIgnitionCycle()`.
- En `SAFE_STATE`, un latched crítico conduce a `SHUTDOWN`.
- Una solicitud normal conduce a `SHUTDOWN_REQ` y espera permiso.
- `SHUTDOWN` es terminal hasta reiniciar `Control`.

## Evidencia

Las pruebas cubren rangos, timeouts, bordes temporales, recuperación, latching,
resumen de fallos, transiciones globales y escenarios señal-a-estado. Consulta
[TESTING.md](TESTING.md).
