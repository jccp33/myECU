# Comportamiento actual

Estado del código revisado el 6 de octubre de 2026. La especificación de las
máquinas de estados se encuentra en [FSM.md](FSM.md).

## Ciclo de procesamiento

1. `MessageManager` inicializa y actualiza los `Message` derivados de `SystemConfig`.
2. `Gateway::validateMessage()` evalúa reloj, timeout y rango y establece `SignalStatus`.
3. `Control::processMessages()` entrega condiciones booleanas a `FaultManager`.
4. Los registros avanzan por `INACTIVE`, `PENDING`, `CONFIRMED`, `RECOVERING` o `LATCHED`.
5. `FaultManager::getSummary()` produce un `FaultSummary`.
6. `Control` entrega entradas de dominio a `EcuStateMachine` y conserva el estado resultante.

El simulador y ambas aplicaciones STM32 (`main` y `control_test`) utilizan
este pipeline. `SignalSample` y `SignalStore` permanecen fuera de él; se
conservan como soporte bajo revisión y con pruebas propias de `SignalStore`.

## Configuración y tiempos

`src/config.cpp` configura once señales, IDs 100–110, incluida presión de
aceite. Todas usan timeout de 500 ms, confirmación de 200 ms y recuperación
de 500 ms. El voltaje es crítico latched; el resto es recuperable.
La temperatura admite actualmente -20 a 130 °C.

Los extremos del rango son inclusivos. El timeout se activa cuando la edad
es mayor que el umbral y tiene prioridad sobre fuera de rango. Una muestra
con timestamp futuro produce `CLOCK_ERROR` y `UNDEFINED`.
La evaluación periódica limita la resolución observable: 500 ms nominales
en la simulación automática y 100 ms en STM32.

## Estados globales y recuperación

La FSM realiza una transición por llamada. `Control` suministra actualmente
`initializationComplete=true` y `selfTestResult=PASSED`; no ejecuta una prueba
física de arranque. El primer ciclo pasa de `INIT` a `SELF_TEST` y el siguiente
resuelve el estado según diagnóstico y fallos.

- Un fallo recuperable se limpia después de permanecer sano durante `recoveryTimeMs`.
- `RECOVERING` sigue contando como fallo activo.
- Un fallo latched requiere `FaultManager::resetForIgnitionCycle()`.
- En `SAFE_STATE`, un crítico latched conduce a `SHUTDOWN`.
- Una solicitud normal conduce a `SHUTDOWN_REQ`, que espera permiso.
- `SHUTDOWN` es terminal hasta reiniciar `Control`; su reset no limpia por sí solo `FaultManager`.

Las prioridades dependen del estado: en operación/degradación, diagnóstico
y crítico prevalecen sobre solicitud; en `SAFE_STATE`, crítico latched y
solicitud preceden al resto. Consulte la tabla de [FSM.md](FSM.md).

## Simulador y registro

El modo manual procesa al terminar la entrada de valores y no supervisa
continuamente mientras espera al usuario. El automático evoluciona señales,
inyecta ventanas aleatorias de rango/timeout y acepta `B` y `S`.

Solo el modo automático utiliza `Logger`: abre `ecu.log` en append, registra
transiciones y enumera fallos activos al entrar en `DEGRADED` o `SAFE_STATE`.
No registra cambios de fallo sin transición global ni comprueba errores de
apertura/escritura. Véanse [SIMULATION.md](SIMULATION.md) y [LOGGING.md](LOGGING.md).

## Aplicaciones STM32

La adquisición actual contempla TPS (PA0), NTC (PA1), MAP (PA2) y RPM (PA6/TIM3).
Una lectura NTC eléctricamente inválida no refresca el mensaje.

`control_test` inicializa RPM y renueva ocho señales nominales por software,
incluida RPM. TPS, NTC y MAP quedan excluidas de esa renovación. El refresco
nominal de RPM puede ocultar pérdida de pulsos. Además, el objeto global
`RPM_SENSOR` necesita un constructor que el startup actual no ejecuta.
La integración física RPM sigue pendiente.

`app/stm32/main.cpp` no inicializa RPM ni renueva las señales restantes.
Con la configuración actual, sus timeouts incluyen voltaje crítico latched
y llevan al apagado. El script de control compila `control_test.cpp`.

## Validación y límites pendientes

CMake y Make compilan CORE y simulador. CMake comprueba cada header público
y permite un perfil freestanding, que no configura por sí solo un toolchain ARM.
Las doce pruebas host pasan mediante `scripts/run_tests.sh`; aún no se
integran en Make/CMake. El firmware `control_test` también se compiló para ARM
sin flashear durante la revisión. Véase [TESTING.md](TESTING.md).

Pendientes conocidos: rechazo de `NaN`, validación completa de configuración
física, tratamiento de señales vencidas como solicitud/permiso de apagado y
paso a estado seguro cuando el conteo de mensajes excede la capacidad.
La regresión de reloj en `FaultManager` se compara con la entrada al estado,
no con la última evaluación. El wrap-around de `millis()` STM32 y las
condiciones compartidas con ISR requieren validación específica.
