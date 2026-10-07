# Implementación STM32F103C8T6 Blue Pill --- myECU

## Propósito

Este documento deja reproducible la implementación física y de software
actualmente utilizada por **myECU** sobre una **STM32F103C8T6 Blue
Pill**. Describe el pipeline vigente basado en `Message`, el hardware ya
validado y las entradas ADC integradas: TPS, temperatura NTC y MAP.
La captura RPM está implementada, pero su integración física con el control
sigue pendiente. Estado documental: 6 de octubre de 2026.

## 1. Plataforma

-   MCU: STM32F103C8T6 Blue Pill (ARM Cortex-M3).
-   Flash considerada por el linker: 64 KiB; SRAM: 20 KiB.
-   Programador/debugger: ST-LINK V2.
-   Host: Ubuntu 24.04.
-   Toolchain: `arm-none-eabi-g++`, `arm-none-eabi-objcopy`, `st-flash`.
-   C++11, bare-metal, sin RTOS.
-   Protoboard, cables Dupont, LEDs externos y resistencias de 220 Ω.
-   TPS de prueba: potenciómetro B103 ≈10 kΩ.
-   Temperatura: NTC 10 kΩ + resistencia fija ≈9.97 kΩ.
-   Analizador lógico Saleae-compatible disponible para validación
    temporal.

## 2. Organización STM32

``` text
core/             lógica portable; no conoce STM32, ADC ni GPIO
app/stm32/        aplicación y adaptación de señales físicas
platform/stm32/   startup, linker, GPIO, ADC, SysTick y runtime
```

Archivos principales:

``` text
app/stm32/
├── main.cpp
├── control_test.cpp
├── signal_acquisition.hpp
├── signal_acquisition.cpp
├── rpm_sensor.{hpp,cpp}
└── rpm_input_test.cpp

platform/stm32/
├── include/{adc.hpp,led.hpp,time.hpp,rpm_input.hpp}
├── src/{adc.cpp,led.cpp,time.cpp,rpm_input.cpp,runtime.cpp}
├── startup/startup_stm32f103.cpp
└── linker/stm32f103c8.ld
```

Para integrar sensores uno por uno se utiliza especialmente
`control_test.cpp`: mantiene nominales las señales todavía no conectadas
y permite aislar la señal física bajo prueba.

## 3. Pipeline funcional vigente

``` text
Sensor físico
 → entrada STM32
 → driver platform
 → signal_acquisition
 → MessageManager
 → Message
 → Gateway
 → SignalStatus
 → Control
 → FaultManager / FaultStateMachine
 → FaultSummary
 → EcuStateMachine
 → EcuState
 → LED físico
```

## 4. Temporización

`platform/stm32/src/time.cpp` configura SysTick para una interrupción
cada 1 ms.

``` text
CPU asumida: 8 MHz (HSI)
SysTick: 1 kHz
Reload: 8000 - 1
Ciclo de aplicación: ~100 ms
```

El tiempo del ciclo se entrega al CORE mediante `TimestampMs`.

## 5. ADC

El driver está en `platform/stm32/src/adc.cpp` y utiliza ADC1.

-   12 bits: 0...4095.
-   Referencia usada en conversiones: 3.3 V.
-   ADC clock: PCLK2/6.
-   Inicio por software y calibración durante inicialización.
-   Tiempo de muestreo configurado explícitamente: 55.5 ciclos en canales 0 y 1.
-   Lectura bloqueante esperando EOC.

  Señal             Pin   ADC      Canal
  ----------------- ----- ------ -------
  TPS               PA0   ADC1         0
  Temperatura NTC   PA1   ADC1         1
  MAP               PA2   ADC1         2

**No aplicar 5 V a PA0, PA1 ni PA2; el montaje actual trabaja con señales
analógicas de hasta 3.3 V.**

## 6. TPS --- SignalId 106

Montaje:

``` text
3.3 V ── extremo B103 10 kΩ
             │
          cursor ───────── PA0 / ADC1_IN0
             │
GND ───── extremo
```

Validación física aproximada:

``` text
0.00 V → ADC ≈ 1
3.28 V → ADC ≈ 4093
```

Conversión:

``` text
V = ADCraw × 3.3 / 4095
```

Configuración:

``` text
SignalId:      1.1.106.0
Señal:         Posición de Mariposa
Rango:         0.5...4.8 V
Severidad:     DEGRADED
Timeout:       500 ms
Confirmación:  200 ms
Recuperación:  500 ms
Latching:      RECOVERABLE
```

Aunque el límite lógico superior es 4.8 V, la entrada física del STM32
se mantiene en 3.3 V como máximo.

Comportamiento validado:

``` text
TPS válido → OPERATIONAL → LED verde
TPS < 0.5 V durante confirmación → DEGRADED → LED amarillo
retorno a rango válido → recuperación
```

## 7. Temperatura NTC --- SignalId 104

Circuito validado:

``` text
3.3 V
  │
[R fija ≈ 9.97 kΩ]
  │
  ├──────── PA1 / ADC1_IN1
  │
[NTC 10 kΩ]
  │
 GND
```

Mediciones observadas:

``` text
ambiente:      ~1.62 V
NTC calentado: ~1.42 V
```

La adquisición acepta únicamente `ADCraw > 100` y `< 4000`. Una lectura
eléctricamente inválida no refresca el `Message`.

Conversión:

``` text
V = ADCraw × 3.3 / 4095
Rntc = 9970 × V / (3.3 - V)
```

Modelo Beta:

``` text
R0 = 10000 Ω
T0 = 298.15 K (25 °C)
Beta = 3950 K

1/T = 1/T0 + (1/Beta) × ln(R/R0)
Tcelsius = Tkelvin - 273.15
```

Configuración:

``` text
SignalId:      1.1.104.0
Señal:         Temperatura
Rango:         -20...130 °C
Severidad:     CRITICAL
Timeout:       500 ms
Confirmación:  200 ms
Recuperación:  500 ms
Latching:      RECOVERABLE
```

Las sesiones previas registraron respuesta física del NTC. La calibración,
la exactitud del modelo Beta y los escenarios diagnósticos con el rango
actual de -20 a 130 °C siguen pendientes de validación completa.

## 7.1. MAP --- SignalId 107

MAP se adquiere como una tercera entrada analógica:

``` text
salida MAP o fuente de prueba 0...3.3 V ─── PA2 / ADC1_IN2
GND de la fuente                         ─── GND común
```

Conversión:

``` text
V = ADCraw × 3.3 / 4095
```

Configuración:

``` text
SignalId:      1.1.107.0
Señal:         Presión Absoluta
Rango:         0.5...4.7 V
Severidad:     DEGRADED
Timeout:       500 ms
Confirmación:  200 ms
Recuperación:  500 ms
Latching:      RECOVERABLE
```

El límite superior lógico no autoriza 4.7 V en PA2: la entrada física debe
permanecer en 3.3 V o menos. Para sensores con salida superior se requiere
acondicionamiento externo.

## 8. LEDs de estado

  EcuState                  LED        Pin
  ------------------------- ---------- -----
  INIT / SELF_TEST          Azul       PB8
  OPERATIONAL               Verde      PB5
  DEGRADED                  Amarillo   PB6
  SAFE_STATE                Rojo       PB7
  SHUTDOWN_REQ / SHUTDOWN   apagados   ---

Los GPIO del montaje son active-high. Cada LED externo usa resistencia
limitadora de 220 Ω. Ya se validaron físicamente `OPERATIONAL → verde` y
`DEGRADED → amarillo`.

## 9. Señales configuradas

     ID Señal                    Rango         Severidad
  ----- ------------------------ ------------- -----------
    100 Solicitud de apagado     0--1          WARNING
    101 Solicitud de freno       0--1          WARNING
    102 Velocidad                0--220 km/h   DEGRADED
    103 RPM                      0--7000 rpm   CRITICAL
    104 Temperatura              -20--130 °C    CRITICAL
    105 Voltaje                  8--16 V       CRITICAL
    106 TPS                      0.5--4.8 V    DEGRADED
    107 MAP / Presión absoluta   0.5--4.7 V    DEGRADED
    108 MAF                      2--120 g/s    DEGRADED
    109 Oxígeno                  0.1--0.9 V    DEGRADED
    110 Presión de aceite        1--6 bar      CRITICAL

Entradas físicas STM32 actuales:

``` text
104 → Temperatura NTC → PA1 / ADC1_IN1
106 → TPS             → PA0 / ADC1_IN0
107 → MAP             → PA2 / ADC1_IN2
```

## 10. `control_test`

`app/stm32/control_test.cpp` es el firmware recomendado para incorporar
sensores uno por uno. `refreshNominalSignals()` actualiza con valores
nominales ocho señales, incluida RPM; excluye TPS, temperatura y MAP.
Esto permite aislar las entradas ADC de timeouts ajenos, pero enmascara
la pérdida de pulsos RPM. La adquisición física se ejecuta después del
refresco nominal.

El orden de cada ciclo de 100 ms es:

``` text
refrescar ocho señales nominales (incluida RPM)
→ adquirir TPS, temperatura, MAP e intentar adquirir RPM
→ validar rango/timeout con Gateway
→ procesar fallos y estado global
→ mostrar EcuState mediante LEDs
```

### RPM y aplicación principal

RPM utiliza PA6/TIM3_CH1, captura de período y conversión mediante `RpmSensor`
con un pulso por revolución y máximo provisional de 7000 RPM.
`control_test` llama a `initRpmInput()`, pero el objeto global `RPM_SENSOR`
necesita construcción dinámica y el startup no ejecuta inicializadores C++.
Debe cerrarse ese punto y eliminarse el enmascaramiento nominal antes de
validar pérdida de pulsos y recuperación. `rpm_input_test.cpp` crea su
conversor dentro de `main`; su script permite una prueba aislada.

`app/stm32/main.cpp` usa el mismo pipeline, pero no inicializa RPM ni renueva
las señales sin adquisición. Con la configuración actual, estas vencen y el
voltaje crítico latched conduce a apagado. No es el archivo compilado por
`stm32_control_test.sh`.

El logger de archivos pertenece al simulador Linux y no se compila para STM32.

## 11. Compilación y programación

Ejecutar:

``` bash
./scripts/stm32_control_test.sh
```

El script compila Cortex-M3 con C++11/`-Og -g3`, modo freestanding, sin
excepciones ni RTTI, define `MYECU_MAX_SENSOR_COUNT=16`, enlaza con
`stm32f103c8.ld`, genera ELF/BIN/MAP y programa mediante `st-flash`.

Artefactos:

``` text
build-stm32/control_test.elf
build-stm32/control_test.bin
build-stm32/control_test.map
```

Dirección Flash:

``` text
0x08000000
```

## 12. Procedimiento de reproducción

1.  Conectar Blue Pill y ST-LINK por SWD según el pinout de ambos
    dispositivos y mantener tierra común.
2.  Montar LEDs externos con 220 Ω: PB5 verde, PB6 amarillo, PB7 rojo,
    PB8 azul.
3.  Montar TPS: B103 entre 3.3 V/GND, cursor a PA0.
4.  Montar NTC: 3.3 V → 9.97 kΩ → nodo PA1 → NTC 10 kΩ → GND.
5.  Conectar MAP o una fuente de prueba limitada a 0...3.3 V en PA2.
6.  Antes de conectar una señal analógica al MCU, verificar con
    multímetro que no exceda 3.3 V.
7.  Ejecutar `./scripts/stm32_control_test.sh`.
8.  Mantener TPS y MAP por encima de 0.5 V y la temperatura entre -20 y
    130 °C; verificar `OPERATIONAL`/verde.
9.  Llevar TPS por debajo de 0.5 V durante más de 200 ms →
    DEGRADED/amarillo.
10. Restaurar TPS durante más de 500 ms y verificar recuperación.
11. Repetir el caso degradado llevando MAP por debajo de 0.5 V.
12. Verificar que la temperatura estimada cambia coherentemente al
    variar físicamente la temperatura del NTC.
13. Para validar un fallo de temperatura, utilizar una fuente de prueba
    controlada que produzca una estimación fuera de -20...130 °C y mantenerla
    más de 200 ms; verificar `SAFE_STATE` y recuperación tras volver al rango.
    Calentar por encima de 30 °C ya no supera el umbral actual. Una lectura
    ADC eléctricamente inválida prueba timeout, no directamente fuera de rango.

## 13. Estado alcanzado

``` text
Blue Pill STM32F103C8T6
├── startup bare-metal + linker
├── SysTick / millis
├── GPIO: PB5/PB6/PB7/PB8
├── ADC1
│   ├── PA0 → TPS
│   ├── PA1 → NTC temperatura
│   └── PA2 → MAP
├── adquisición física
├── MessageManager / Message
├── Gateway
├── FaultManager
├── Control
├── EcuStateMachine
└── indicación física de EcuState
```

## 14. Próximos pasos

- Validar físicamente MAP a través de PA2 y documentar mediciones.
- Calibrar el modelo NTC y validar el rango actual de -20 a 130 °C.
- Definir acondicionamiento para señales que excedan 3.3 V.
- Cerrar la integración física RPM (constructor, refresco nominal, PPR y timeout).
- Incorporar adquisición de velocidad y definir el alcance de `main.cpp`.
- Añadir pruebas de timeout, simultaneidad y recuperación para las tres
  entradas físicas.
