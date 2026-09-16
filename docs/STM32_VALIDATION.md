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
3.3 V ─── extremo A
             |
          [ 10 kΩ ]
             |
PA0  ───── cursor
             |
          [ 10 kΩ ]
             |
GND  ───── extremo B
```

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
