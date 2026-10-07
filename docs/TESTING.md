# Pruebas y evidencia de validación

## Suite host vigente

Desde la raíz del repositorio:

```bash
./scripts/run_tests.sh
# Compilador alternativo, si está instalado:
CXX=clang++ ./scripts/run_tests.sh
```

El script compila cada `tests/*.cpp` con C++11 y advertencias, las fuentes del
CORE y soporte de configuración, simulación y `SignalStore`. Guarda resultados
en `build-tests/` y termina ante un error de compilación o una prueba fallida.
No está integrado en CMake, CTest ni Make.

En la revisión del 6 de octubre de 2026 se ejecutaron correctamente los doce
programas siguientes:

| Ejecutable | Cobertura principal |
|---|---|
| `control_tests` | Arranque, degradado, crítico, señal indefinida y reset |
| `diagnostic_status_tests` | Conversión de errores y disponibilidad diagnóstica |
| `ecu_integration_tests` | Pipeline nominal, confirmación y recuperación de velocidad |
| `ecu_state_machine_test` | Transiciones globales, prioridades, apagado y diagnóstico |
| `evaluation_rule_tests` | Política derivada y validez de regla |
| `fault_configuration_tests` | Derivación, duplicados, capacidad y divergencia de política |
| `fault_manager_tests` | Gestión por señal, resumen, latching, reloj y reset |
| `fault_state_machine_tests` | Umbrales de confirmación/recuperación y latching |
| `fault_types_tests` | Identidad y tipos de dominio |
| `gateway_tests` | Rango inclusivo, límites de timeout y regresión de reloj |
| `sensor_simulation_tests` | Evolución gradual y señales discretas |
| `signal_store_tests` | Identidad, actualización, timestamps y capacidad fija |

`SignalStore` tiene pruebas propias, pero no forma parte del pipeline vigente.
Puede repetirse un ejecutable construido, por ejemplo:

```bash
./build-tests/gateway_tests
./build-tests/ecu_integration_tests
```

## Compilación y comprobaciones de headers

La revisión compiló el simulador y el CORE con CMake sin advertencias visibles.
CMake compila aisladamente los once headers públicos del CORE. Esto comprueba
includes y compatibilidad de compilación; no es una suite funcional.

```bash
cmake -S . -B build-cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-cmake
```

`make sanitize` construye el simulador con ASan/UBSan. No ejecuta la suite,
y compilar con sanitizadores no demuestra ausencia de errores en ejecución.
El roadmap conserva la verificación previa con estos instrumentos; no se
repitió una sesión de sanitizadores durante esta actualización documental.

## Logger e interfaz del simulador

El logger compila con el simulador. `ecu.log` contiene registros de arranque,
degradación, recuperación, apagado solicitado y apagado por crítico latched.
Son observaciones de simulación, no pruebas automatizadas del logger.

La suite no cubre específicamente apertura/escritura/cierre de archivos,
fallos de E/S, append, formato de eventos ni la integración de `Logger` con
el bucle de consola. Tampoco cubre todas las ventanas de inyección automática,
la entrada interactiva o semillas repetibles. Véase [LOGGING.md](LOGGING.md).

## Compilación ARM

Se compiló `control_test` con los argumentos del script STM32, sin ejecutar
su fase de programación. Resultado: `text=8428`, `data=0`, `bss=60` bytes.
Estas cifras no incluyen la pila ni miden memoria máxima en ejecución.

Los scripts `stm32_control_test.sh` y `stm32_rpm_input_test.sh` compilan,
generan el binario y programan la placa; no ofrecen una opción de solo build.
La compilación ARM no equivale a una nueva validación física.

## Evidencia histórica STM32

Las sesiones registradas documentan startup, linker, SWD, GPIO, SysTick,
ejecución del CORE, estados `OPERATIONAL`/`SAFE_STATE`, ciclo aproximado de
100 ms, medición con analizador lógico y diagnóstico/recuperación de TPS con
LEDs. Sus resultados y alcance se conservan en
[STM32_VALIDATION.md](STM32_VALIDATION.md) y
[ANALOG_INPUT_SESSION.md](ANALOG_INPUT_SESSION.md).
La configuración reproducible actual está en
[BLUE_PILL_IMPLEMENTATION.md](BLUE_PILL_IMPLEMENTATION.md).

## Pendientes de cobertura

- `NaN`, límites físicos inválidos y configuración completa de señales.
- Señales vencidas utilizadas como solicitud/permiso de apagado.
- Conteo de mensajes inválido y mensajes ausentes del ciclo.
- Regresión temporal respecto a la última evaluación y wrap-around STM32.
- Integración física RPM, constructor global y timeout sin refresco nominal.
- Calibración NTC y validación física completa de MAP.
- ISR, atomicidad, captura coincidente con overflow y datos compartidos.
- Stack máximo, WCET, jitter, watchdog e inyección eléctrica de fallos.
- Estrategia SIL/PIL/HIL para reproducir los casos host sobre hardware.
