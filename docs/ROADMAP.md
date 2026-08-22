# Checklist — Convertir `myECU` en un Portable ECU Core

## Objetivo

Transformar el `myECU` actual desde una aplicación de simulación Linux hacia una arquitectura donde el mismo núcleo lógico pueda ejecutarse posteriormente sobre:

```text
Linux Simulator
      │
      ▼
  ECU Core
      ▲
      │
STM32 / NXP / MCU
```

El objetivo de esta etapa **no es implementar CAN, UDS ni FreeRTOS todavía**.

El criterio de éxito será:

> `Message`, `Gateway` y `Control` pueden compilarse, probarse y ejecutarse sin depender de consola, POSIX, `std::chrono`, teclado, generación aleatoria ni hardware específico.

---

# FASE 1 — Congelar el comportamiento actual

Antes de refactorizar, necesitamos poder demostrar que no rompemos lo que ya funciona.

Comportamiento congelado: [Comportamiento actual del ECU Core](CURRENT_BEHAVIOR.md).

## 1. Documentar los escenarios actuales de funcionamiento

* [x] Documentar comportamiento ante una señal no crítica fuera de rango.
* [x] Identificar la secuencia necesaria para llegar a `INIT`.
* [x] Documentar transición `INIT → SELF_TEST`.
* [x] Documentar transición `SELF_TEST → OPERATIONAL`.
* [x] Documentar transición `OPERATIONAL → DEGRADED`.
* [x] Documentar recuperación `DEGRADED → OPERATIONAL`.
* [x] Documentar transición `OPERATIONAL → SAFE_STATE`.
* [x] Documentar recuperación `SAFE_STATE → OPERATIONAL`.
* [x] Documentar transición `SAFE_STATE → SHUTDOWN`.
* [x] Documentar comportamiento ante `TIMEOUT`.
* [x] Documentar comportamiento ante una señal crítica fuera de rango.

### Resultado esperado

Debes terminar con algo equivalente a:

```text
INPUT                               EXPECTED STATE

startup                             INIT
system initialized                  SELF_TEST
valid self test                     OPERATIONAL
non-critical sensor failure         DEGRADED
sensor recovery                     OPERATIONAL
critical sensor failure             SAFE_STATE
vehicle stopped + shutdown          SHUTDOWN
```

### Debes poder explicar

* [x] Qué evento provoca cada transición.
* [x] Qué condición actúa como `guard`.
* [x] Qué estados permiten recuperación.
* [x] Qué estados deberían ser terminales.

---

# FASE 2 — Preparar el proyecto para detectar errores

## 2. Endurecer los warnings del compilador

Mantener:

```text
-Wall
-Wextra
-Wpedantic
```

Agregar progresivamente:

```text
-Wshadow
-Wconversion
-Wsign-conversion
-Wold-style-cast
-Wnull-dereference
-Wdouble-promotion
```

### Tareas

* [x] Compilar con cada warning adicional.
* [x] Revisar uno por uno los warnings.
* [x] No silenciar warnings sin entenderlos.
* [x] Corregir conversiones implícitas peligrosas.
* [x] Corregir variables que oculten otras variables.
* [x] Eliminar casts estilo C en C++.
* [x] Revisar comparaciones `signed` vs `unsigned`.

### Criterio de terminado

```text
build = successful
warnings relevantes = 0
```

---

# FASE 3 — Activar detección de Undefined Behavior

## 3. Agregar sanitizers

Crear una configuración de desarrollo usando:

```text
-fsanitize=address,undefined
-fno-omit-frame-pointer
-g
```

### Tareas

* [x] Compilar `myECU` con AddressSanitizer.
* [x] Ejecutar simulación manual.
* [x] Ejecutar simulación aleatoria.
* [x] Forzar estados de error.
* [x] Ejecutar durante varios minutos.
* [x] Revisar accesos fuera de rango.
* [x] Revisar objetos después de terminar su lifetime.
* [x] Revisar null dereferences.
* [x] Revisar integer UB.

Después:

* [x] Ejecutar UBSan.
* [x] Documentar cualquier problema encontrado. ASan/UBSan ejecutados: sin problemas detectados.
* [x] Corregirlo antes de avanzar.

### Criterio de terminado

Todas las rutas principales pueden ejecutarse sin reportes de:

```text
ASan
UBSan
```

---

# FASE 4 — Corregir el modelo básico de C++

Esta etapa conecta directamente con lo que necesitas reforzar de C++.

## 4. Revisar `const-correctness`

Revisar cada clase.

Especialmente:

```text
Message
Gateway
Control
MessageManager
```

Por cada función preguntarte:

> ¿Esta función modifica el objeto?

Si la respuesta es no:

```cpp
return_type function(...) const;
```

### Tareas

* [x] Marcar getters como `const`.
* [x] Revisar parámetros `T&`.
* [x] Convertir a `const T&` los parámetros que solo se leen.
* [x] Revisar funciones que reciben strings.
* [x] Revisar funciones que retornan strings.
* [x] Evitar copias innecesarias.

### Debes poder explicar

Diferencia entre:

```cpp
Message*
const Message*
Message* const
const Message* const

Message&
const Message&
```

---

# FASE 5 — Revisar lifetime y ownership

## 5. Hacer un mapa de ownership

Para cada objeto principal:

```text
MessageManager
Gateway
Control
Message
SystemConfig
```

documentar:

```text
¿Quién lo crea?
¿Dónde vive?
¿Quién lo posee?
¿Quién puede modificarlo?
¿Cuándo muere?
```

Ejemplo:

```text
Control

created by: main/application
storage: automatic
owner: application
borrowed by: simulation
destroyed: scope exit
```

### Tareas

* [x] Identificar objetos con automatic storage.
* [x] Identificar objetos con static storage.
* [x] Identificar allocations dinámicas, si existen.
* [x] Verificar que no exista `new/delete` innecesario.
* [x] Determinar dónde RAII ya resuelve lifetime automáticamente.

### Regla

No introducir `shared_ptr` ni `unique_ptr` solo para practicar.

Solo utilizarlos cuando exista ownership dinámico real.

---

# FASE 6 — Eliminar dependencias de plataforma del Core

Este es el cambio arquitectónico principal.

## 6. Clasificar archivos actuales

Clasificar cada archivo como:

```text
CORE
PLATFORM
SIMULATION
```

Ejemplo conceptual:

```text
CORE
----
message
gateway
control
system configuration

SIMULATION
----------
random simulation
user simulation

PLATFORM
--------
terminal
clock
sleep
keyboard
```

### Tareas

* [ ] Crear una tabla archivo → responsabilidad.
* [x] Detectar cualquier `std::cout` dentro del Core.
* [x] Detectar cualquier `std::cin` dentro del Core.
* [x] Detectar `std::chrono` dentro del Core.
* [x] Detectar POSIX dentro del Core.
* [x] Detectar random generators dentro del Core.
* [x] Detectar `sleep` dentro del Core.

### Criterio

Los módulos:

```text
Message
Gateway
Control
```

no deben depender de nada específico de Linux.

---

# FASE 7 — Extraer el reloj

## 7. El Core no debe consultar la hora

Actualmente cualquier función de tiempo utilizada por la lógica debe dejar de obtener el tiempo directamente.

En vez de:

```cpp
auto now = get_timestamp_ms();
```

preferir conceptualmente:

```cpp
processMessage(message, currentTimeMs);
```

### Tareas

* [x] Identificar todos los lugares que consultan tiempo.
* [x] Determinar cuáles pertenecen realmente al Core.
* [x] Modificar APIs para recibir timestamp explícitamente.
* [x] Definir el tipo usado para tiempo.

Ejemplo:

```cpp
using TimestampMs = std::uint64_t;
```

* [x] Evitar que `Message`, `Gateway` o `Control` conozcan `std::chrono`.
* [x] Mantener `std::chrono` solamente en LinuxPlatform/Simulator.

### Resultado

Linux:

```text
std::chrono
    ↓
timestamp
    ↓
ECU Core
```

Futuro MCU:

```text
hardware timer
    ↓
timestamp
    ↓
ECU Core
```

---

# FASE 8 — Extraer la salida de consola

## 8. Eliminar presentación de la lógica de ECU

Buscar funciones como:

```text
printCurrentState()
getColorsMessage()
std::cout
```

### Tareas

* [x] Identificar dónde el dominio imprime directamente.
* [x] Separar información de presentación.
* [x] Hacer que `Control` solo mantenga/cambie estado.
* [x] Permitir consultar:

```cpp
EcuState getState() const;
```

* [x] Mover impresión de estados a la simulación.
* [x] Mover códigos ANSI de colores fuera del Core.
* [x] Mantener el Core completamente usable sin terminal.

### Arquitectura objetivo

```text
Control
   │
   └── returns EcuState
              │
              ▼
       LinuxConsoleView
```

---

# FASE 9 — Definir una API mínima de ECU

Cuando el Core esté limpio, definir una interfaz central.

No necesariamente crear todavía una clase gigante `ECU`.

Primero identificar operaciones.

Conceptualmente:

```text
ecuInit()
ecuProcessInput()
ecuUpdate()
ecuState()
```

### Tareas

* [ ] Definir qué necesita el Core para inicializarse.
* [ ] Definir qué constituye una entrada.
* [ ] Definir cómo se procesa una señal.
* [ ] Definir cómo se consulta el estado.
* [ ] Definir qué información debe salir del Core.

Una posible dirección conceptual:

```cpp
class EcuCore {
public:
    void initialize(...);

    void processMessage(
        const Message& message,
        TimestampMs now
    );

    EcuState state() const;
};
```

No copies esta interfaz automáticamente.

Primero decide si realmente mejora tu arquitectura actual.

---

# FASE 10 — Introducir eventos explícitos

Actualmente muchas decisiones están basadas directamente en valores de `Message`.

Empieza a estudiar una separación:

```text
raw signal
   ↓
Gateway
   ↓
validated signal / event
   ↓
Control
```

### Tareas

* [x] Identificar cuáles condiciones pertenecen a `Gateway`.
* [x] Identificar cuáles pertenecen a `Control`.
* [x] Evitar duplicar thresholds.
* [x] Evitar que `Control` vuelva a validar datos físicos ya validados.
* [x] Definir una representación clara de fallo.

Por ejemplo conceptualmente:

```text
SignalStatus::VALID
SignalStatus::OUT_OF_RANGE
SignalStatus::TIMEOUT
```

y posteriormente quizá:

```text
SignalEvent
```

### Pregunta obligatoria

Para cada condición:

> ¿Esto es validación de una señal o decisión de control?

---

# FASE 11 — Eliminar números mágicos

## 11. Centralizar configuración

Buscar:

```text
11.0
500
6
1000
etc.
```

cuando tengan significado de sistema.

### Tareas

* [ ] Identificar thresholds duplicados.
* [ ] Identificar timeout hardcoded.
* [ ] Identificar número de sensores hardcoded.
* [ ] Identificar periodos hardcoded.
* [ ] Llevar configuración a `SystemConfig` o equivalente.
* [ ] Utilizar constantes nombradas cuando corresponda.

### Resultado esperado

Una única fuente de verdad para:

```text
sensor ID
minimum
maximum
criticality
timeout
unit
```

---

# FASE 12 — Revisar contenedores

## 12. Eliminar construcciones no estándar o innecesarias

Especialmente arrays cuyo tamaño no sea compile-time.

### Tareas

* [x] Revisar todos los arrays built-in.
* [x] Eliminar cualquier VLA.
* [x] Utilizar `std::array` si el tamaño es fijo.
* [x] Utilizar `std::vector` si el tamaño es runtime.
* [x] Justificar cada contenedor.

Debes poder responder:

```text
¿Por qué array?
¿Por qué vector?
¿Por qué no list?
¿Por qué tamaño fijo?
```

---

# FASE 13 — Crear los primeros unit tests

Esta es una etapa obligatoria antes del hardware.

## 13. Instalar/configurar framework de tests

Elegir uno:

```text
GoogleTest
Catch2
```

Para aprendizaje profesional, GoogleTest sería una opción razonable.

### Primer conjunto: Gateway

* [x] Mensaje dentro de rango → `VALID`.
* [x] Mensaje bajo mínimo → `OUT_OF_RANGE`.
* [x] Mensaje sobre máximo → `OUT_OF_RANGE`.
* [x] Mensaje expirado → `TIMEOUT`.
* [x] Mensaje exactamente en mínimo.
* [x] Mensaje exactamente en máximo.

---

# FASE 14 — Unit tests de `Control`

## 14. Probar cada transición

* [x] Construir un `Control` limpio.
* [x] Confirmar estado inicial.
* [x] Test `INIT → SELF_TEST`.
* [x] Test `SELF_TEST → OPERATIONAL`.
* [x] Test `OPERATIONAL → DEGRADED`.
* [x] Test `DEGRADED → OPERATIONAL`.
* [x] Test `OPERATIONAL → SAFE_STATE`.
* [x] Test `SAFE_STATE → OPERATIONAL`.
* [x] Test `SAFE_STATE → SHUTDOWN`.

### También probar

* [x] Señales inválidas múltiples.
* [x] Fallo crítico seguido de recuperación.
* [x] Fallo no crítico seguido de fallo crítico.
* [x] Mensaje con SensorId inválido.
* [x] Repetición del mismo mensaje.
* [x] Estado `SHUTDOWN` ante mensajes posteriores.

### Criterio

No depender de:

```text
sleep
keyboard
random
real clock
```

para ninguno de los tests.

---

# FASE 15 — Tests deterministas de timeout

Esta fase comprueba si la abstracción de tiempo quedó bien diseñada.

Ejemplo conceptual:

```text
message timestamp = 1000 ms
current time      = 1499 ms

→ VALID
```

```text
message timestamp = 1000 ms
current time      = 2001 ms

→ TIMEOUT
```

### Tareas

* [x] Crear casos de borde.
* [x] Probar exactamente el timeout.
* [x] Probar timeout + 1.
* [x] Probar timeout - 1.
* [x] Evitar cualquier `sleep()`.

Si necesitas esperar tiempo real para probar `Gateway`, la arquitectura todavía no está suficientemente desacoplada.

---

# FASE 16 — Crear `LinuxPlatform`

Ahora sí reconstruir la simulación sobre el Core.

Conceptualmente:

```text
LinuxPlatform
├── clock
├── console
├── keyboard
└── random sensor generator
```

### Tareas

* [x] Mover código POSIX aquí.
* [ ] Mover `std::chrono` aquí.
* [ ] Mover generación aleatoria aquí.
* [ ] Mover `sleep_for` aquí.
* [ ] Mover colores ANSI aquí.
* [x] Mantener comportamiento visible actual.

### Criterio

La aplicación Linux debe seguir funcionando igual que antes.

Pero internamente:

```text
LinuxPlatform
      ↓
ECU Core
```

---

# FASE 17 — Crear una estructura de carpetas más representativa

Una posible evolución:

```text
myECU/
│
├── core/
│   ├── include/
│   └── src/
│
├── platform/
│   └── linux/
│
├── simulator/
│
├── tests/
│
├── docs/
│
├── CMakeLists.txt
└── README.md
```

No necesitas usar exactamente estos nombres.

La regla importante es que **Core y plataforma sean visibles arquitectónicamente**.

### Tareas

* [ ] Separar fuentes Core.
* [ ] Separar fuentes Linux.
* [x] Separar tests.
* [ ] Ajustar includes.
* [x] Verificar que Core compile independientemente.

---

# FASE 18 — Migrar gradualmente a CMake

No elimines inmediatamente el Makefile.

## 18. Crear CMake básico

* [x] Crear target `ecu_core`.
* [x] Crear target `ecu_simulator`.
* [x] Linkear simulator → core.
* [x] Crear target de tests.
* [x] Integrar CTest con los ejecutables de prueba existentes.
* [x] Integrar warnings.
* [x] Crear configuración Debug.
* [x] Crear configuración Release.

Arquitectura de targets:

```text
ecu_core
   ▲
   │
ecu_simulator

ecu_core
   ▲
   │
ecu_tests
```

Esta estructura es importante porque prueba que `ecu_core` no depende del simulador.

---

# FASE 19 — Ejecutar análisis estático

## 19. Incorporar herramientas

* [ ] Ejecutar `clang-tidy`.
* [ ] Ejecutar `cppcheck`.
* [ ] Revisar warnings individualmente.
* [ ] No aplicar automáticamente todas las sugerencias.
* [ ] Documentar qué recomendaciones rechazas y por qué.

### Temas que observar

```text
const correctness
narrowing
copies
lifetime
nullability
unused code
switch exhaustiveness
```

---

# FASE 20 — Medir memoria del Core

Ya tienes análisis inicial en el README.

Ahora separar:

```text
Simulator
vs
ECU Core
```

### Tareas

* [ ] Medir `sizeof(Message)`.
* [ ] Medir `sizeof(Control)`.
* [ ] Medir configuración.
* [ ] Medir almacenamiento de sensores.
* [ ] Identificar allocations dinámicas.
* [ ] Determinar si el Core necesita heap.
* [ ] Documentar memoria estática aproximada.

### Pregunta importante

> ¿Podría funcionar el Core sin ninguna allocation dinámica después de initialization?

Para embedded, esa sería una propiedad interesante.

---

# FASE 21 — Fuera de alcance: Ring Buffer externo

El repositorio o carpeta `STRUCTS` es un proyecto independiente y no forma parte
del alcance ni del cálculo de avance de `myECU`.

Si el milestone embedded requiere posteriormente un ring buffer, se diseñará o
integrará dentro del alcance de `myECU`, con requisitos y pruebas propios. Esa
decisión corresponde a la fase de comunicación posterior al primer milestone
físico.

---

# FASE 22 — Diseñar la entrada física futura

Antes de comprar/integrar hardware, definir qué quieres demostrar.

Primera ECU física mínima:

```text
Potentiometer
     │
     ▼
    ADC
     │
     ▼
  TPS value
     │
     ▼
 ECU Core
     │
     ▼
LED / output
```

### Tareas

* [ ] Elegir una señal analógica.
* [ ] Definir rango eléctrico.
* [ ] Definir rango digital.
* [ ] Definir conversión ADC → valor físico.
* [ ] Definir qué condición será válida.
* [ ] Definir qué condición provocará fallo.
* [ ] Definir salida observable.

---

# FASE 23 — Estudiar arquitectura embedded mínima

Antes del MCU dominar conceptualmente:

* [ ] superloop;
* [ ] polling;
* [ ] interrupt;
* [ ] periodic execution;
* [ ] hardware timer;
* [ ] GPIO;
* [ ] ADC;
* [ ] memory-mapped registers;
* [ ] `volatile`;
* [ ] startup básico;
* [ ] cross compilation.

Debes poder explicar la diferencia entre:

```text
while(true)
```

y:

```text
timer interrupt
```

y cuándo usar cada uno.

---

# FASE 24 — Elegir MCU

No elijas por cantidad de características.

Para esta etapa necesitas:

```text
GPIO
ADC
timer
UART
CAN posteriormente
debugger
```

### Tareas

* [ ] Comparar STM32 Nucleo.
* [ ] Comparar una alternativa NXP.
* [ ] Verificar disponibilidad de CAN.
* [ ] Verificar debugger integrado.
* [ ] Verificar documentación.
* [ ] Verificar toolchain.
* [ ] Elegir una sola plataforma.

---

# FASE 25 — Crear `MCUPlatform`

Cuando Core sea estable:

```text
platform/
├── linux/
└── stm32/
```

o equivalente.

### Primera implementación

Solo:

* [ ] clock;
* [ ] GPIO;
* [ ] ADC.

Nada de CAN todavía.

### Resultado

```text
ADC
 ↓
MCUPlatform
 ↓
ECU Core
 ↓
MCUPlatform
 ↓
LED
```

---

# FASE 26 — Primer milestone físico

## `myECU Embedded 0.1`

Debe demostrar:

* [ ] compila para MCU;
* [ ] arranca;
* [ ] lee ADC real;
* [ ] convierte ADC a una señal;
* [ ] entrega la señal al mismo Core;
* [ ] Gateway valida;
* [ ] Control actualiza FSM;
* [ ] GPIO refleja estado;
* [ ] `OPERATIONAL` observable;
* [ ] `DEGRADED` observable;
* [ ] `SAFE_STATE` observable.

### Definición de éxito

Puedes cambiar entre:

```text
Linux Simulator
```

y:

```text
MCU Firmware
```

manteniendo esencialmente intactos:

```text
Message
Gateway
Control
SystemConfig
```

Cuando eso funcione, habrás alcanzado el objetivo:

# Portable ECU Core

---

# FASE 27 — Solo después: comunicación real

Después del milestone anterior:

```text
UART logging
    ↓
Ring Buffer propio o adaptado para myECU
    ↓
CAN
    ↓
SocketCAN interoperability
    ↓
multiple ECUs
    ↓
DBC
    ↓
ISO-TP
    ↓
UDS
    ↓
RTOS si existe necesidad real
```

No invertiría ese orden.

---

# Estado de avance

Corte verificado: **22 de agosto de 2026**.

El cálculo excluye la Fase 21 asociada al proyecto externo `STRUCTS` y utiliza
cada checkbox de las fases técnicas de `myECU` como una unidad de trabajo:

```text
Tareas completadas: 112
Tareas en alcance:   182
Avance técnico:     61.54 %
```

La secuencia inmediata lleva **14 de 18 tareas (77.78 %)**.

---

# Orden inmediato de trabajo

Si mañana continúas `myECU`, no empieces por las 27 fases simultáneamente.

Tu siguiente secuencia concreta es:

* [x] **Tarea 1:** hacer inventario de dependencias de plataforma.
* [x] **Tarea 2:** clasificar archivos como Core / Simulator / Platform.
* [x] **Tarea 3:** eliminar acceso directo al reloj desde el Core.
* [x] **Tarea 4:** eliminar `cout`/colores/presentación del Core.
* [x] **Tarea 5:** aplicar `const-correctness`.
* [x] **Tarea 6:** revisar arrays/contenedores/lifetime.
* [x] **Tarea 7:** activar warnings fuertes + ASan + UBSan.
* [x] **Tarea 8:** escribir tests de `Gateway`.
* [x] **Tarea 9:** escribir tests de `Control`.
* [x] **Tarea 10:** hacer tests deterministas de timeout.
* [x] **Tarea 11:** crear target/biblioteca `ecu_core`.
* [x] **Tarea 12:** reconstruir la simulación Linux sobre `ecu_core`.
* [x] **Tarea 13:** comprobar que `ecu_core` puede compilar sin POSIX.
* [x] **Tarea 14:** migrar targets a CMake.
* [ ] **Tarea 15:** elegir MCU.
* [ ] **Tarea 16:** portar únicamente clock + GPIO + ADC.
* [ ] **Tarea 17:** ejecutar la misma FSM con una señal física.
* [ ] **Tarea 18:** documentar resultados como `myECU Embedded 0.1`.

No avanzar a CAN hasta que la **Tarea 17** funcione.

Ese es el camino más corto desde el estado actual de `myECU` hacia un sistema embedded implementable, y mantiene la progresión correcta de aprendizaje: **C/C++ profundo → memoria/UB → pruebas/debugging → abstracción de plataforma → MCU/periféricos → comunicación/RTOS**.
