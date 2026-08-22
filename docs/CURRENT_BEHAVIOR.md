# Comportamiento actual del ECU Core

Este documento congela el comportamiento observable de `Gateway` y `Control`
antes de continuar con cambios arquitectónicos. Describe lo que implementa el
código actual, aunque alguna regla todavía no represente el comportamiento final
deseado para una ECU real.

## Flujo de una señal

```text
valor + timestamp
       |
       v
    Gateway  -----> SignalStatus
       |              VALID
       |              OUT_OF_RANGE
       |              TIMEOUT
       v
    Control  -----> EcuState
```

`Gateway` valida primero el tiempo y después el rango. `Control` conserva el
último estado recibido de cada señal, identificada por `Message::messageId`, y
recalcula el estado global de la ECU.

## Validación realizada por Gateway

Para esta descripción:

```text
elapsed = currentTimeMs - message.timestampMs
```

Se presupone un reloj monotónico y `currentTimeMs >= timestampMs`.

| Condición | Resultado |
|---|---|
| `elapsed > timeoutMs` | `TIMEOUT` |
| `elapsed <= timeoutMs` y `minValue <= rawValue <= maxValue` | `VALID` |
| `elapsed <= timeoutMs` y valor fuera del intervalo cerrado | `OUT_OF_RANGE` |

El timeout tiene prioridad sobre el rango. En el instante exacto del timeout el
mensaje todavía no está expirado; expira un milisegundo después.

## Estados y transiciones de Control

| Estado actual | Evento | Guard o condición | Estado resultante |
|---|---|---|---|
| construcción o `reset()` | ninguno | siempre | `INIT` |
| `INIT` | primer `processMessage()` | cualquier mensaje y cualquier `SignalStatus` | `SELF_TEST` |
| `SELF_TEST` | siguiente `processMessage()` | todas las señales registradas están en `VALID` | `OPERATIONAL` |
| `SELF_TEST` | siguiente `processMessage()` | hay fallos no críticos y su cantidad no supera el límite | `DEGRADED` |
| `SELF_TEST` | siguiente `processMessage()` | existe un fallo crítico o la cantidad de fallos supera el límite | `SAFE_STATE` |
| `OPERATIONAL` | mensaje inválido no crítico | fallos `<= maxInvalidSignals` | `DEGRADED` |
| `DEGRADED` | actualización de las señales fallidas | todas las señales registradas vuelven a `VALID` | `OPERATIONAL` |
| `OPERATIONAL` o `DEGRADED` | mensaje inválido crítico | existe al menos un fallo crítico | `SAFE_STATE` |
| cualquier estado no terminal | procesamiento de mensajes | fallos `> maxInvalidSignals` | `SAFE_STATE` |
| `SAFE_STATE` | actualización de las señales fallidas | todas las señales registradas vuelven a `VALID` | `OPERATIONAL` |
| `DEGRADED` o `SAFE_STATE` calculado | solicitud de apagado activa | `isShutdownRequest == true` y `rawValue == activeValue` | `SHUTDOWN` |
| `SHUTDOWN` | cualquier mensaje posterior | siempre | `SHUTDOWN` |

El límite usa una comparación estricta: con
`maxInvalidSignals == 4`, cuatro fallos no críticos mantienen `DEGRADED` y el
quinto provoca `SAFE_STATE`.

## Secuencia nominal de arranque

```text
1. Construir Control                     -> INIT
2. Procesar el primer mensaje            -> SELF_TEST
3. Procesar otro mensaje con todos
   los registros conocidos en VALID      -> OPERATIONAL
```

`INIT` no se alcanza mediante una señal. Es el estado establecido por el
constructor y por `Control::reset()`.

El primer mensaje siempre provoca `INIT -> SELF_TEST`; su validez no actúa como
guard en esa transición. El Core tampoco comprueba actualmente que durante el
self-test se haya recibido el conjunto completo de sensores configurados: solo
evalúa los registros conocidos hasta ese momento.

## Escenarios congelados

| Entrada o escenario | Precondición | Resultado esperado |
|---|---|---|
| Arranque | construir `Control` | `INIT` |
| Sistema inicializado | procesar primer mensaje | `SELF_TEST` |
| Self-test válido | todas las señales registradas son `VALID` | `OPERATIONAL` |
| Señal no crítica bajo el mínimo o sobre el máximo | `OPERATIONAL` | `DEGRADED` |
| Recuperación de señal no crítica | `DEGRADED`; todas vuelven a `VALID` | `OPERATIONAL` |
| Señal crítica bajo el mínimo o sobre el máximo | `OPERATIONAL` | `SAFE_STATE` |
| Recuperación de señal crítica | `SAFE_STATE`; todas vuelven a `VALID` | `OPERATIONAL` |
| `TIMEOUT` no crítico | `OPERATIONAL`; fallos dentro del límite | `DEGRADED` |
| `TIMEOUT` crítico | `OPERATIONAL` o `DEGRADED` | `SAFE_STATE` |
| Solicitud activa de apagado durante un fallo crítico persistente | estado calculado `SAFE_STATE` | `SHUTDOWN` |
| Mensaje posterior al apagado | `SHUTDOWN` | permanece en `SHUTDOWN` |

Para `Control`, `TIMEOUT` y `OUT_OF_RANGE` son estados inválidos equivalentes al
contar fallos. La criticidad de la señal es la que determina si un único fallo
lleva a `DEGRADED` o a `SAFE_STATE`.

## Recuperación y estados terminales

- `DEGRADED` permite recuperación directa a `OPERATIONAL`.
- `SAFE_STATE` permite recuperación directa a `OPERATIONAL`.
- `INIT` y `SELF_TEST` son estados transitorios durante el arranque.
- `SHUTDOWN` es terminal frente a `processMessage()`: ningún mensaje posterior
  cambia el estado.
- `reset()` es una operación administrativa y puede llevar cualquier estado,
  incluido `SHUTDOWN`, nuevamente a `INIT`.

## Limitaciones conocidas del comportamiento actual

Estas observaciones son deliberadamente parte del comportamiento congelado; no
deben corregirse de manera incidental durante otro refactor:

1. No existe un guard de velocidad cero para entrar a `SHUTDOWN`. La implementación
   actual no consulta la señal `SPEED` durante esa transición.
2. Una solicitud activa no apaga desde `OPERATIONAL` si todas las señales son
   válidas. El estado calculado debe ser `DEGRADED` o `SAFE_STATE`.
3. Una vez detectada, la solicitud de apagado queda almacenada hasta `reset()`;
   una actualización posterior con valor inactivo no la cancela.
4. El primer mensaje lleva a `SELF_TEST` incluso si está en `TIMEOUT` o
   `OUT_OF_RANGE`.
5. `Control` identifica registros por `messageId`; actualmente no rechaza un
   valor de `SensorId` fuera del enum conocido.

## Evidencia automatizada

Los escenarios están cubiertos por:

- `tests/gateway_tests.cpp`: rango, extremos y bordes deterministas de timeout.
- `tests/control_tests.cpp`: arranque, transiciones, recuperaciones, acumulación
  de fallos, apagado y estado terminal.

Se ejecutan mediante:

```bash
ctest --test-dir build-cmake-debug --output-on-failure
```

