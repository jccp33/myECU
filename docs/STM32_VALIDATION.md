# STM32 Hardware Validation

## Objetivo

Validar que el CORE portable de myECU puede ejecutarse sobre hardware STM32
sin depender de la plataforma Linux, usando una plataforma bare-metal mínima.

La plataforma validada actualmente es:

- MCU: STM32F103C8T6
- Arquitectura: ARM Cortex-M3
- Flash detectada: 64 KiB
- SRAM detectada: 20 KiB
- Programador/debugger: ST-LINK V2
- Toolchain: `arm-none-eabi-g++`
- Host de desarrollo: Ubuntu 24.04
- Analizador lógico: Saleae-compatible FX2, 8 canales
- Software de captura: PulseView / sigrok

## Estructura STM32

```text
myECU/
├── app/
│   └── stm32/
│       └── main.cpp
│
├── platform/
│   └── stm32/
│       ├── include/
│       │   ├── led.hpp
│       │   └── time.hpp
│       ├── src/
│       │   ├── led.cpp
│       │   ├── time.cpp
│       │   └── runtime.cpp
│       ├── startup/
│       │   └── startup_stm32f103.cpp
│       └── linker/
│           └── stm32f103c8.ld
│
├── include/
└── src/
```

## Validación de entrada analógica ADC

### Objetivo

Validar una entrada analógica física sobre el STM32F103C8T6 antes de integrarla con el core portable de myECU.

La prueba utiliza un potenciómetro de 10 kΩ como fuente analógica controlable para verificar la cadena:

```text
Potenciómetro
    ↓
Voltaje analógico
    ↓
PA0 / ADC1_IN0
    ↓
ADC1
    ↓
platform::readAdc()
    ↓
Valor digital de 12 bits
```

### Conexión utilizada

El potenciómetro se conectó como divisor de voltaje:

```text
3.3 V ───── extremo A
               │
               │
          ┌────┴────┐
          │   10 kΩ │
          │   POT   │
          └────┬────┘
               │
GND ────── extremo B
               ↕ cursor
               │
               └──────── PA0 / ADC1_IN0
```

El potenciómetro utilizado está marcado como `B103`, correspondiente a un
valor nominal de 10 kΩ entre sus dos extremos.

El cursor no representa una resistencia adicional de 10 kΩ. Su posición
divide la resistencia total del potenciómetro en dos partes variables cuya
suma permanece aproximadamente en 10 kΩ.

Durante la validación se midieron aproximadamente 10.5 kΩ entre los extremos
del componente.

La entrada analógica se conectó a `PA0`, correspondiente a `ADC1_IN0`.

Se utilizó exclusivamente alimentación de 3.3 V para mantener la entrada dentro del rango permitido por el STM32F103.

### Configuración del ADC

Se agregó la interfaz:

```text
platform/stm32/include/adc.hpp
platform/stm32/src/adc.cpp
```

La implementación configura:

- GPIOA / PA0 en modo analógico.
- ADC1.
- Canal 0 (`ADC1_IN0`).
- Conversión de 12 bits.
- Tiempo de muestreo.
- Calibración del ADC.
- Inicio de conversión mediante software.

### Resultado experimental

Las mediciones físicas del cursor del potenciómetro fueron aproximadamente:

```text
Posición mínima: 0.00 V
Posición máxima: 3.28 V
```

Mediante GDB se inspeccionó directamente el valor retornado por el ADC:

```text
0.00 V  → ADC = 1
3.28 V  → ADC = 4093
```

Para un ADC de 12 bits, el intervalo teórico es:

```text
0 ... 4095
```

Por lo tanto, la entrada física cubrió prácticamente todo el rango disponible del ADC.

**Resultado: PASS**

---

## Integración de entrada analógica TPS con el core de myECU

### Objetivo

Integrar la entrada analógica previamente validada con la arquitectura de señales y diagnóstico de myECU.

La cadena validada fue:

```text
Potenciómetro
    ↓
PA0 / ADC1
    ↓
platform::readAdc()
    ↓
Conversión ADC → voltaje
    ↓
SignalSample
TPS / SignalId 106
    ↓
SignalStore
    ↓
FaultManager
    ↓
Regla RANGE del TPS
```

### Capa de adquisición

Se agregaron:

```text
app/stm32/signal_acquisition.hpp
app/stm32/signal_acquisition.cpp
```

Esta capa mantiene separadas las responsabilidades de la arquitectura:

```text
platform/stm32
    Hardware específico del STM32
    ADC → valor raw

app/stm32
    Adquisición y adaptación
    ADC raw → magnitud física → SignalSample

core
    Procesamiento independiente de plataforma
    SignalStore → FaultManager → Control
```

El core no necesita conocer `PA0`, `ADC1` ni registros específicos del STM32.

### Conversión ADC a voltaje

La conversión implementada es:

```text
V = ADCraw × 3.3 / 4095
```

El valor obtenido se almacena como una muestra:

```text
SignalId: 1.1.106.0
Señal: TPS
Unidad: V
Validity: VALID
Timestamp: millis()
```

La adquisición se ejecuta cada 100 ms y la muestra se introduce en `SignalStore` mediante `upsert()`.

### Regla de diagnóstico utilizada

La configuración existente del TPS define aproximadamente:

```text
Rango válido:       0.5 V ... 4.8 V
Severidad:          DEGRADED
Confirmación:       200 ms
Recuperación:       500 ms
Error:              OUT_OF_RANGE
Tipo:               SENSOR
Latching:           RECOVERABLE
```

El límite superior pertenece a la configuración existente de myECU. En esta prueba, el potenciómetro alimentado con 3.3 V alcanza aproximadamente 3.28 V, suficiente para validar el comportamiento normal y provocar intencionalmente una falla por debajo de 0.5 V.

### Instrumentación temporal de depuración

Para observar directamente el estado de la regla RANGE del TPS se utilizó temporalmente:

```cpp
volatile std::uint8_t g_tpsFaultState = 0U;
```

y se consultó el registro correspondiente mediante:

```cpp
const FaultRecord* tpsFaultRecord =
    faultManager.getRecord(12U);
```

El índice `12` corresponde a la regla RANGE del TPS dentro del conjunto de reglas utilizado durante esta validación.

Esta instrumentación se utilizó únicamente para observación mediante GDB.

### Validación experimental

#### TPS dentro del rango válido

Con el potenciómetro en una posición válida se observó:

```text
g_tpsFaultState = 0
```

Correspondiente a:

```text
FaultState::INACTIVE
```

**Resultado: PASS**

#### TPS fuera de rango

El potenciómetro se llevó al extremo previamente medido en aproximadamente:

```text
0.00 V
```

y se mantuvo durante un tiempo superior a los 200 ms de confirmación.

GDB mostró:

```text
g_tpsFaultState = 2
```

Correspondiente a:

```text
FaultState::CONFIRMED
```

**Resultado: PASS**

#### Recuperación

Posteriormente, el potenciómetro se regresó a aproximadamente:

```text
3.28 V
```

y se mantuvo durante un tiempo superior a los 500 ms de recuperación.

GDB mostró:

```text
g_tpsFaultState = 0
```

Correspondiente nuevamente a:

```text
FaultState::INACTIVE
```

**Resultado: PASS**

### Secuencia observada

La secuencia observada directamente mediante GDB fue:

```text
INACTIVE → CONFIRMED → INACTIVE
   0           2           0
```

Los estados intermedios `PENDING` y `RECOVERING` forman parte de la máquina de estados implementada, pero no fueron observados directamente durante esta prueba.

### Resultado

La integración:

```text
Hardware
    ↓
ADC STM32
    ↓
Capa platform
    ↓
Capa de adquisición
    ↓
SignalSample
    ↓
SignalStore
    ↓
FaultManager
```

fue validada satisfactoriamente sobre hardware real.

**Resultado general: PASS**

### Alcance de esta validación

Esta prueba valida específicamente la ruta:

```text
TPS físico → ADC → SignalSample → SignalStore → FaultManager
```

No debe interpretarse todavía como una validación aislada de:

```text
TPS → FaultManager → Control → EcuState
```

porque actualmente solo el TPS se adquiere físicamente, mientras que las demás señales configuradas pueden generar fallas de timeout al no existir todavía muestras para ellas.

La siguiente validación deberá comprobar de manera controlada la propagación de la falla del TPS hacia `Control` y `EcuState` sin que las señales aún no implementadas interfieran con el resultado.

## Validación de integración TPS → Control → EcuState

### Objetivo

Validar en hardware que una entrada analógica física pueda propagarse a través
de la arquitectura completa de diagnóstico y control de `myECU`, provocando un
cambio observable en el estado global de la ECU.

La cadena validada fue:

```text
Potenciómetro
    ↓
PA0 / ADC1_IN0
    ↓
platform::readAdc()
    ↓
Conversión ADC → voltaje
    ↓
SignalSample (TPS, ID 106)
    ↓
SignalStore
    ↓
FaultManager
    ↓
FaultSummary
    ↓
EcuStateInputs
    ↓
Control
    ↓
EcuState
```

### Estrategia de prueba

Para aislar el efecto del TPS sobre el sistema se creó la aplicación de
validación:

```text
app/stm32/control_test.cpp
```

El TPS (`SignalId 106`) permaneció como una entrada física adquirida mediante
el ADC.

Las otras nueve señales configuradas en `myECU` fueron insertadas
periódicamente en `SignalStore` con valores nominales y timestamps
actualizados. De esta forma se evitó que señales todavía no implementadas
físicamente generaran fallas `RANGE` o `TIMEOUT` que pudieran interferir con
la prueba.

El ciclo de procesamiento se ejecutó cada 100 ms.

La arquitectura utilizada durante la prueba fue:

```text
       Señales simuladas nominales
       100–105, 107–109
                │
                │
                ▼
             SignalStore
                ▲
                │
                │
       TPS físico / ID 106
                ▲
                │
        signal_acquisition
                ▲
                │
             ADC1 / PA0
                ▲
                │
          Potenciómetro
                │
                ▼
            FaultManager
                │
                ▼
           FaultSummary
                │
                ▼
              Control
                │
                ▼
             EcuState
```

### Instrumentación de validación

Para observar directamente el resultado mediante GDB se utilizaron variables
globales `volatile` dentro de la aplicación de prueba:

```cpp
volatile bool g_hasDegraded = false;
volatile bool g_hasCriticalActive = false;
volatile std::uint8_t g_ecuState = 0U;
volatile std::uint8_t g_tpsFaultState = 0U;
```

La regla RANGE del TPS fue inspeccionada mediante:

```cpp
const FaultRecord* tpsFaultRecord =
    faultManager.getRecord(12U);

if (tpsFaultRecord != nullptr)
{
    g_tpsFaultState =
        static_cast<std::uint8_t>(tpsFaultRecord->state);
}
```

El índice `12` corresponde a la regla RANGE del TPS dentro del
`EvaluationRuleSet` utilizado durante esta validación.

Esta instrumentación pertenece exclusivamente a la aplicación de prueba y no
forma parte de la interfaz de producción del core.

### Caso 1: TPS fuera de rango

Con el TPS en una condición inferior al límite mínimo configurado de `0.5 V`,
se dejó ejecutar el sistema durante un tiempo suficiente para superar el
tiempo de confirmación de la regla.

GDB mostró:

```text
g_tpsFaultState     = 2
g_hasDegraded       = true
g_hasCriticalActive = false
g_ecuState          = 3
```

De acuerdo con las enumeraciones del core:

```text
FaultState 2 = CONFIRMED
EcuState   3 = DEGRADED
```

La propagación observada fue:

```text
TPS fuera de rango
        ↓
FaultState::CONFIRMED
        ↓
FaultSummary.hasDegraded = true
        ↓
EcuStateInputs
        ↓
Control::processInputs()
        ↓
EcuState::DEGRADED
```

**Resultado: PASS**

### Caso 2: recuperación del TPS

Posteriormente se llevó el potenciómetro nuevamente a una posición dentro del
rango válido y se dejó ejecutar el firmware durante un intervalo superior al
tiempo de recuperación configurado de `500 ms`.

GDB mostró:

```text
g_tpsFaultState     = 0
g_hasDegraded       = false
g_hasCriticalActive = false
g_ecuState          = 2
```

De acuerdo con las enumeraciones del core:

```text
FaultState 0 = INACTIVE
EcuState   2 = OPERATIONAL
```

La propagación observada fue:

```text
TPS recuperado
        ↓
FaultState::INACTIVE
        ↓
FaultSummary.hasDegraded = false
        ↓
EcuStateInputs
        ↓
Control::processInputs()
        ↓
EcuState::OPERATIONAL
```

**Resultado: PASS**

### Secuencia global validada

Las dos condiciones observadas permiten establecer la siguiente secuencia:

```text
TPS fuera de rango
        ↓
CONFIRMED
        ↓
hasDegraded = true
        ↓
DEGRADED


TPS recuperado
        ↓
INACTIVE
        ↓
hasDegraded = false
        ↓
OPERATIONAL
```

Durante ambas observaciones:

```text
g_hasCriticalActive = false
```

Por lo tanto, el cambio a `DEGRADED` fue coherente con la severidad configurada
para la regla RANGE del TPS y no con una falla crítica.

Los estados intermedios de la máquina de estados de fallas, como `RECOVERING`,
no fueron observados directamente mediante GDB durante esta prueba. La
documentación registra únicamente los estados comprobados experimentalmente.

### Resultado

La integración completa:

```text
Entrada analógica física
        ↓
ADC STM32
        ↓
platform::readAdc()
        ↓
Conversión a voltaje
        ↓
SignalSample
        ↓
SignalStore
        ↓
FaultManager
        ↓
FaultSummary
        ↓
EcuStateInputs
        ↓
Control
        ↓
EcuState
```

fue validada satisfactoriamente sobre el STM32F103C8T6.

Se comprobó experimentalmente que una condición física fuera de rango en el
TPS puede llevar la ECU a `EcuState::DEGRADED` y que, después de recuperar la
señal y completar el tiempo de recuperación, la ECU retorna a
`EcuState::OPERATIONAL`.

**Resultado general: PASS**

### Alcance de esta validación

Esta prueba valida una señal física real, TPS, a través de la cadena completa
de adquisición, diagnóstico y control de `myECU`.

Las demás señales fueron simuladas con valores nominales y timestamps
actualizados para aislar el TPS. Por lo tanto, esta prueba no demuestra todavía
la adquisición física simultánea de todos los sensores configurados.

La aplicación `control_test.cpp`, sus variables globales de observación y el
acceso directo al índice `12` constituyen infraestructura de validación y no
deben interpretarse como interfaces definitivas de producción.

## Physical ECU State Indication with External LEDs

### Objective

Validate that the ECU state calculated by the portable `myECU` core can be
observed directly on physical hardware using external LEDs.

This validation extends the previous TPS ADC integration by connecting the
resulting `EcuState` to physical GPIO outputs on the STM32F103C8T6 Blue Pill.

### Hardware

- STM32F103C8T6 Blue Pill
- ST-LINK V2
- B103 10 kOhm potentiometer
- Breadboard
- External LEDs
- 220 Ohm current-limiting resistors
- Jumper wires

### LED mapping

| ECU state | LED | STM32 pin |
|---|---|---|
| `INIT` / `SELF_TEST` | Blue | PB8 |
| `OPERATIONAL` | Green | PB5 |
| `DEGRADED` | Yellow | PB6 |
| `SAFE_STATE` | Red | PB7 |
| `SHUTDOWN_REQ` / `SHUTDOWN` | All off | - |

The LED driver belongs to the STM32 platform layer. The portable core does not
depend on GPIOs, LEDs, STM32 registers, or any other platform-specific
implementation.

The application layer maps the portable `EcuState` to the corresponding
platform LED.

### Validated signal path

The complete physical path validated during this test was:

    Potentiometer
        |
        v
    STM32 ADC (PA0)
        |
        v
    TPS SignalSample
        |
        v
    SignalStore
        |
        v
    FaultManager
        |
        v
    FaultSummary
        |
        v
    Control
        |
        v
    EcuState
        |
        v
    External state LED

The potentiometer therefore acts as a real physical TPS input rather than a
simulated signal.

### Observed behavior

With the TPS voltage inside its accepted operating range, the ECU reaches:

    EcuState::OPERATIONAL

and the green LED is activated.

When the potentiometer is moved into the configured TPS fault region and the
fault satisfies the diagnostic timing requirements, the ECU reaches:

    EcuState::DEGRADED

and the yellow LED is activated.

This demonstrates that a physical analog input can propagate through the
complete diagnostic and control architecture and produce an externally
observable ECU state.

### Breadboard intermittency incident

During validation, the original breadboard assembly developed intermittent
behavior. Symptoms included unstable behavior when the breadboard was moved
and ADC behavior that appeared to be inconsistent with potentiometer
position.

Software, ADC configuration, and ECU state handling were investigated during
diagnosis.

The physical circuit was subsequently rebuilt using:

- a different breadboard,
- a different B103 10 kOhm potentiometer,
- and a revised jumper-wire arrangement.

Before reconnecting the MCU, the replacement potentiometer was validated with
a multimeter:

    P0 <-> P2: approximately 10.36-10.37 kOhm
    P0 <-> P1: approximately 0-10.36 kOhm across the knob travel

After rebuilding the circuit, the expected ECU state transitions were observed
again.

The exact defective component or contact in the original assembly was not
isolated. Therefore, the result should be interpreted as evidence of a
physical interconnection problem in the previous assembly, rather than proof
that a specific breadboard, potentiometer, or jumper was defective.

### Diagnostic lesson

When future ADC measurements appear stuck, unstable, or inconsistent with a
physical sensor input, the hardware signal path should be verified before
changing firmware.

Recommended checks include:

1. Sensor resistance or output.
2. Supply voltage and ground.
3. Sensor output voltage.
4. Breadboard continuity.
5. Jumper continuity and mechanical contact.
6. ADC input voltage at the MCU pin.
7. ADC register/sample value.
8. ECU diagnostic state.

This separates physical signal-integrity problems from software defects.

### Architectural significance

The external LEDs are now more than a GPIO validation mechanism. They provide
a simple physical indication of the global ECU state.

As additional physical signals are integrated, each signal can contribute to
the existing diagnostic architecture:

    Physical signals
          |
          v
      SignalStore
          |
          v
     FaultManager
          |
          v
     FaultSummary
          |
          v
       Control
          |
          v
       EcuState
          |
          v
     State LEDs

This does not require the portable core to know anything about the LEDs or the
STM32 platform.

The same `EcuState` can later be exposed through other platform-specific
interfaces such as UART, CAN, diagnostic messages, or telemetry without
changing the core state-machine architecture.

### Validation status

**Status: PHYSICALLY VALIDATED**

Validated on STM32F103C8T6 Blue Pill with a real potentiometer used as the TPS
analog input.

Observed physical state indication:

- `OPERATIONAL` -> green LED
- `DEGRADED` -> yellow LED
