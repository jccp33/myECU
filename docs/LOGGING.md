# Registro de eventos del simulador

## Alcance

`Logger` está implementado en `include/logger.hpp` y `src/logger.cpp`.
Pertenece al soporte de host y se utiliza desde `randomSimulation()` en
`src/simulations.cpp`. Make y CMake incluyen su fuente en el simulador;
`ecu_core` y el firmware STM32 no dependen del logger.

El modo automático registra cambios del estado global y, al entrar en
`DEGRADED` o `SAFE_STATE`, los registros de fallo activos. El modo manual
no utiliza `Logger`.

## Uso

Desde la raíz del proyecto, con el simulador compilado:

```bash
./ecu -auto
```

Para observar el registro desde otra terminal:

```bash
tail -f ecu.log
```

El archivo se abre como `ecu.log` en el directorio de trabajo del proceso.
Se utiliza modo append (`"a"`): se crea si no existe y se conservan los
registros de ejecuciones anteriores. Los archivos `*.log` están ignorados
por Git. No hay rotación ni límite de tamaño.

## Formato

Ejemplo tomado del registro del simulador:

```text
[4380437 ms] OPERATIONAL -> DEGRADED
    SIGNAL: Posicion de Mariposa | STATUS: fuera de tiempo | FAULT: CONFIRMED | VALUE: 1.42 V
[4390508 ms] DEGRADED -> OPERATIONAL
```

La línea de transición contiene timestamp, estado anterior y estado nuevo.
El timestamp procede de `get_timestamp_ms()`, basado en `std::chrono::steady_clock`:
no es fecha/hora de calendario ni tiempo contado desde el inicio de la
simulación. Su origen depende de la implementación del reloj.

Después de una transición a `DEGRADED` o `SAFE_STATE`, `logFaultCauses()`
recorre las señales configuradas e incluye las que tienen un registro en
`CONFIRMED`, `RECOVERING` o `LATCHED`. Cada línea muestra nombre, estado
inmediato de señal, estado temporal del fallo y último valor con dos decimales
y su unidad. Se incluyen todos esos fallos activos, no solo los responsables
de la transición. Un registro `RECOVERING` puede tener una señal ya válida;
un `LATCHED` puede persistir después de desaparecer la condición física.
En un timeout, el valor mostrado es la última muestra almacenada.

## Ciclo de vida e interfaz

- `Logger()` inicializa el archivo como cerrado.
- `open(filename)` devuelve si pudo abrirlo; devuelve `false` si ya está abierto.
- `write(message)` agrega una línea y llama a `fflush()` después de escribir.
- `close()` cierra el archivo; el destructor también llama a `close()`.

`write()` no hace nada si el archivo está cerrado o el mensaje es nulo.
Actualmente `randomSimulation()` no comprueba el resultado de `open()`;
si la apertura falla, la simulación continúa sin registro. Los resultados de
`fprintf()`, `fflush()` y `fclose()` no se propagan al llamador.

El vaciado con `fflush()` facilita observar las líneas durante la ejecución;
no garantiza persistencia ante una interrupción de energía. El cierre mediante
destructor corresponde a una salida normal de la función, como el retorno al
alcanzar `SHUTDOWN`; no debe asumirse ante terminación abrupta del proceso.

## Límites actuales

- Solo se escribe cuando cambia `EcuState`; no se registra cada ciclo.
- No hay eventos separados de inicio/fin de sesión ni identificador de ejecución.
- Un fallo que aparece o cambia sin modificar el estado global no genera línea.
- No se registran individualmente `PENDING`, confirmación o recuperación si
  no hay transición global; `INIT` inicial tampoco tiene una línea propia.
- Un `SAFE_STATE` provocado por diagnóstico interno puede no tener líneas
  de causas: estas se obtienen exclusivamente de los registros de `FaultManager`.
- No hay DTC persistentes, freeze-frames ni registro de todas las muestras.
- La clase posee un `FILE*` y no bloquea la copia. Debe usarse como objeto local
  sin copiar ni asignar; el uso actual en la simulación cumple esa condición.
- No hay pruebas automatizadas específicas del logger ni de su integración
  con el bucle interactivo. Las líneas existentes son evidencia de ejecución.

Posibles mejoras pendientes: manejo explícito de errores de E/S, impedir copias,
identificar sesiones y registrar cambios de fallo sin transición global.
