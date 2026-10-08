# Ruta de aprendizaje: Sistemas Embebidos y Robótica

## Objetivos

1.  Desarrollar mis propios robots.
2.  Aprender sistemas embebidos y robótica mediante proyectos prácticos.
3.  Comprender los periféricos y algoritmos utilizados, evitando depender de abstracciones que oculten su funcionamiento.
4.  Construir progresivamente una plataforma robótica propia.

------------------------------------------------------------------------

# Etapa 0 --- Cerrar myECU

**Objetivo:** terminar correctamente el proyecto que ya proporcionó la base de sistemas embebidos.

-   [ ] Retomar integración física de RPM.
-   [ ] Instrumentar `TIM3 → readRpmPeriodUs() → RpmSensor`.
-   [ ] Resolver el constructor global RPM no ejecutado por el startup.
-   [ ] Excluir RPM del refresco nominal que oculta pérdida de pulsos.
-   [ ] Validar `RPM → Message 103`.
-   [ ] Validar `VALID → OPERATIONAL`.
-   [ ] Validar pérdida de pulsos → `TIMEOUT`.
-   [ ] Validar `TIMEOUT → CRITICAL → SAFE_STATE`.
-   [ ] Validar recuperación de RPM → `OPERATIONAL`.
-   [ ] Resolver/documentar PPR del encoder.
-   [x] Actualizar documentación del estado actual y logger (6 de octubre de 2026).
-   [ ] Mantener documentación al cerrar la integración física RPM.
-   [ ] Documentar arquitectura final.
-   [ ] Crear/taggear **myECU v1.0**.
-   [ ] Congelar alcance: no seguir agregando sensores innecesariamente.

El simulador automático ya dispone de registro de transiciones y fallos
activos en `logs/txt/`, o muestras por ciclo en `logs/csv/` con `-auto -csv`;
la selección aleatoria excluye señales `LATCHED`. Véase [LOGGING.md](LOGGING.md).
Según la validación previa, las doce pruebas host
pasan por script dedicado; aún falta cobertura del logger y validación RPM
en hardware. Esta actualización documental no cierra esos pendientes.

**Criterio de salida:** myECU queda como una primera ECU experimental terminada y documentada.

------------------------------------------------------------------------

# Etapa 1 --- Generación de señales: SG90 + PWM

**Hardware:** NUCLEO-F411RE + SG90 + analizador lógico.

**Objetivo:** pasar de medir señales a generar señales físicas deterministas.

## Lección 1.1 --- Anatomía del SG90

-   [ ] Entender alimentación, GND y señal.
-   [ ] Entender qué contiene internamente un servo.
-   [ ] Diferenciar servo de motor DC y stepper.
-   [ ] Estudiar período y ancho de pulso.
-   [ ] Entender por qué PWM de servo no equivale simplemente a duty cycle.
-   [ ] Diseñar alimentación segura del SG90.

## Lección 1.2 --- Timers STM32 como salida

-   [ ] Estudiar contador, prescaler y ARR.
-   [ ] Calcular frecuencia del timer.
-   [ ] Configurar un timer manualmente.
-   [ ] Configurar Output Compare/PWM.
-   [ ] Generar aproximadamente 50 Hz.
-   [ ] Generar pulsos de diferentes anchos.

Ecuaciones que debemos poder derivar:

``` text
f_timer = f_clk / (PSC + 1)

f_PWM = f_timer / (ARR + 1)
```

## Lección 1.3 --- Primer driver

Crear una API aproximadamente así:

``` cpp
class Servo
{
public:
    void setAngle(float degrees);
};
```

-   [ ] Separar driver hardware de lógica de aplicación.
-   [ ] Convertir grados → microsegundos.
-   [ ] Convertir microsegundos → timer ticks.
-   [ ] Saturar comandos fuera de rango.
-   [ ] Evitar `delay()` para generar pulsos.

## Lección 1.4 --- Validación física

Con el analizador lógico:

-   [ ] Medir período.
-   [ ] Medir pulse width.
-   [ ] Comprobar 0°.
-   [ ] Comprobar 90°.
-   [ ] Comprobar 180° o límite físico real.
-   [ ] Medir jitter.
-   [ ] Comparar cálculo teórico con señal real.

## Lección 1.5 --- Varios servos

-   [ ] Controlar 2 SG90.
-   [ ] Controlar 4 SG90.
-   [ ] Diseñar una abstracción reutilizable.
-   [ ] Investigar límites de corriente.
-   [ ] Evitar alimentar varios servos desde la Nucleo.

**Proyecto final:** controlador bare-metal multicanal de servos.

**Criterio de salida:** poder explicar desde el reloj del STM32 hasta el movimiento físico del servo.

------------------------------------------------------------------------

# Etapa 2 --- Sensado de movimiento: encoder incremental

**Hardware:** STM32F103/NUCLEO-F411RE + encoder + analizador lógico.

**Objetivo:** dominar posición, dirección y velocidad.

## Lección 2.1 --- Quadrature encoding

-   [ ] Conectar CLK/A.
-   [ ] Conectar DT/B.
-   [ ] Observar ambas señales.
-   [ ] Entender desfase de 90°.
-   [ ] Determinar CW/CCW.
-   [ ] Entender PPR/CPR.

## Lección 2.2 --- Implementación software

Primero:

-   [ ] Polling A/B.
-   [ ] Determinar dirección.
-   [ ] Contar posición.

Después:

-   [ ] EXTI.
-   [ ] Comparar polling vs interrupciones.

Finalmente:

-   [ ] STM32 Timer Encoder Mode.
-   [ ] Leer contador directamente del timer.
-   [ ] Manejar overflow/underflow.

## Lección 2.3 --- Velocidad

-   [ ] Medir Δposición/Δt.
-   [ ] Calcular velocidad angular.
-   [ ] Comparar ventana fija vs período entre pulsos.
-   [ ] Analizar resolución a baja velocidad.
-   [ ] Analizar resolución a alta velocidad.

## Lección 2.4 --- Señal real

-   [ ] Observar rebote.
-   [ ] Detectar transiciones inválidas.
-   [ ] Experimentar con filtros del timer.
-   [ ] Medir pérdida de eventos.

**Proyecto final:** módulo reutilizable con una interfaz conceptual:

``` text
Encoder
 ├── position()
 ├── direction()
 ├── velocity()
 └── reset()
```

------------------------------------------------------------------------

# Etapa 3 --- Laboratorio de tiempo real

**Hardware:** NUCLEO-F411RE + analizador lógico.

**Objetivo:** comprender qué significa realmente *real-time*.

## Lección 3.1 --- Polling vs interrupciones

-   [ ] Implementar ambos.
-   [ ] Medir latencia.
-   [ ] Medir carga aproximada.
-   [ ] Comparar comportamiento.

## Lección 3.2 --- NVIC

-   [ ] Prioridades.
-   [ ] Preemption.
-   [ ] Interrupciones anidadas.
-   [ ] ISR largas vs cortas.
-   [ ] Diseñar ISR correctamente.

## Lección 3.3 --- Concurrencia

-   [ ] `volatile`.
-   [ ] Atomicidad.
-   [ ] Race conditions.
-   [ ] Critical sections.
-   [ ] PRIMASK.
-   [ ] Productor/consumidor ISR-main.

## Lección 3.4 --- DMA

-   [ ] Comprender DMA.
-   [ ] ADC + DMA.
-   [ ] UART + DMA.
-   [ ] Comparar CPU-driven vs DMA-driven.

## Lección 3.5 --- Medición temporal

Instrumentación GPIO:

``` text
GPIO HIGH
   ↓
algoritmo
   ↓
GPIO LOW
```

-   [ ] Medir execution time.
-   [ ] Medir ISR latency.
-   [ ] Medir jitter.
-   [ ] Introducción a WCET.

**Proyecto final:** pequeño benchmark documentado del comportamiento temporal del STM32.

------------------------------------------------------------------------

# Etapa 4 --- Primer mecanismo robótico: brazo SG90

**Hardware:** NUCLEO-F411RE + 3--4 SG90.

**Objetivo:** pasar de periféricos aislados a un sistema robótico.

## Lección 4.1 --- Mecánica

-   [ ] Diseñar base.
-   [ ] Hombro.
-   [ ] Codo.
-   [ ] Muñeca/gripper.
-   [ ] Definir límites mecánicos.
-   [ ] Definir sistema de coordenadas.

## Lección 4.2 --- Joint control

Implementar una interfaz conceptual:

``` cpp
robot.setJointAngle(BASE, 45.0F);
robot.setJointAngle(SHOULDER, 70.0F);
```

-   [ ] Límites por articulación.
-   [ ] Conversión ángulo-servo.
-   [ ] Calibración.

## Lección 4.3 --- Trayectorias

En lugar de saltar directamente:

``` text
20° → 120°
```

generar una trayectoria:

``` text
20 → 21 → 22 → ... → 120
```

-   [ ] Velocidad.
-   [ ] Interpolación.
-   [ ] Rampas.
-   [ ] Movimiento simultáneo.

## Lección 4.4 --- Cinemática directa

-   [ ] Sistemas de coordenadas.
-   [ ] Transformaciones.
-   [ ] Matrices de rotación.
-   [ ] Posición del efector.

## Lección 4.5 --- Cinemática inversa

Para un brazo inicialmente planar:

``` text
(x, y) → (θ1, θ2)
```

-   [ ] Resolver geométricamente.
-   [ ] Detectar posiciones imposibles.
-   [ ] Implementar solución en C++.
-   [ ] Validar físicamente.

**Proyecto final:** brazo que recibe coordenadas y mueve el efector hacia ellas.

------------------------------------------------------------------------

# Etapa 5 --- Arquitectura de software + FreeRTOS

**Hardware:** NUCLEO-F411RE.

**Objetivo:** aprender concurrencia embedded sin usar un RTOS como sustituto de un buen diseño.

## Lección 5.1 --- Superloop

Primero construir:

``` cpp
while (true)
{
    updateSensors();
    updateControl();
    updateActuators();
    updateTelemetry();
}
```

-   [ ] Scheduling cooperativo.
-   [ ] Tareas periódicas.
-   [ ] Deadlines.

## Lección 5.2 --- FreeRTOS

-   [ ] Tasks.
-   [ ] Scheduler.
-   [ ] Priorities.
-   [ ] Delays.
-   [ ] Tick.

## Lección 5.3 --- Comunicación

-   [ ] Queues.
-   [ ] Semaphores.
-   [ ] Mutexes.
-   [ ] Task notifications.

## Lección 5.4 --- Problemas reales

-   [ ] Race conditions.
-   [ ] Deadlock.
-   [ ] Priority inversion.
-   [ ] Starvation.
-   [ ] Stack sizing.

## Lección 5.5 --- Arquitectura

-   [ ] Comparar Superloop vs RTOS.
-   [ ] Justificar cuándo utilizar cada arquitectura.

**Proyecto final:** controlador multitarea de sensores + servos + telemetría.

------------------------------------------------------------------------

# Etapa 6 --- Primer robot móvil 2WD

**Hardware adicional previsto:** dos motorreductores con encoder, ruedas, driver, alimentación y estructura.

**Objetivo:** dominar locomoción móvil.

## Lección 6.1 --- Motor DC

-   [ ] Modelo básico.
-   [ ] Torque.
-   [ ] RPM.
-   [ ] Gear ratio.
-   [ ] Back-EMF.
-   [ ] Stall current.

## Lección 6.2 --- Puente H

-   [ ] Dirección.
-   [ ] PWM.
-   [ ] Frenado/coasting.
-   [ ] Protección.
-   [ ] Alimentación separada lógica/motor.

## Lección 6.3 --- Encoder de motor

Aplicar lo aprendido en la Etapa 2:

``` text
motor → gearbox → wheel
          ↓
       encoder
```

-   [ ] RPM real.
-   [ ] Dirección.
-   [ ] Distancia recorrida.

## Lección 6.4 --- PID de velocidad

Conceptos:

``` text
error = velocidad_referencia - velocidad_medida
```

-   [ ] Control P.
-   [ ] Control PI.
-   [ ] Control PID.
-   [ ] Saturación.
-   [ ] Anti-windup.
-   [ ] Ajuste experimental.

## Lección 6.5 --- Differential drive

-   [ ] Velocidad izquierda/derecha.
-   [ ] Movimiento recto.
-   [ ] Rotación.
-   [ ] Radio de giro.

## Lección 6.6 --- Odometría

Estimar:

``` text
x
y
θ
```

a partir de encoders.

**Proyecto final:** robot 2WD que recibe comandos de velocidad y mantiene velocidades de rueda mediante feedback.

------------------------------------------------------------------------

# Etapa 7 --- IMU y sensor fusion

**Hardware:** NUCLEO-F446RE + IMU.

**Objetivo:** estimar orientación a partir de sensores inerciales.

## Lección 7.1 --- I²C

-   [ ] START/STOP.
-   [ ] Address.
-   [ ] ACK/NACK.
-   [ ] Registros.
-   [ ] Driver I²C.

## Lección 7.2 --- IMU

-   [ ] Acelerómetro.
-   [ ] Giroscopio.
-   [ ] Escalas.
-   [ ] Bias.
-   [ ] Ruido.
-   [ ] Calibración.

## Lección 7.3 --- Ángulo mediante acelerómetro

-   [ ] Calcular inclinación.
-   [ ] Observar ruido.
-   [ ] Observar sensibilidad a aceleraciones externas.

## Lección 7.4 --- Ángulo mediante giroscopio

Implementar integración:

``` text
θ[k] = θ[k-1] + ω[k] · Δt
```

-   [ ] Medir drift.

## Lección 7.5 --- Filtro complementario

Implementar:

``` text
θ = α(θ + ω·Δt) + (1-α)θ_acc
```

-   [ ] Ajustar α.
-   [ ] Medir frecuencia.
-   [ ] Analizar respuesta.

## Lección 7.6 --- Kalman

Solo después de comprender los métodos anteriores:

-   [ ] Modelo de estado.
-   [ ] Covarianza.
-   [ ] Prediction.
-   [ ] Correction.
-   [ ] Comparar con filtro complementario.

**Proyecto final:** módulo que entrega orientación estimada en tiempo real.

------------------------------------------------------------------------

# Etapa 8 --- Robot autoequilibrado / péndulo invertido

**Hardware:** NUCLEO-F446RE + IMU + motores con encoder + driver + chasis.

**Objetivo:** integrar sensado, estimación y control en un robot dinámicamente inestable.

## Lección 8.1 --- Construcción mecánica

-   [ ] Chasis.
-   [ ] Dos ruedas.
-   [ ] Centro de masa.
-   [ ] Posición de batería.
-   [ ] IMU rígidamente montada.

## Lección 8.2 --- Modelo

Variables iniciales:

``` text
x
ẋ
θ
θ̇
```

-   [ ] Derivar/estudiar dinámica.
-   [ ] Identificar parámetros físicos.
-   [ ] Relacionarlo con el proyecto académico previo de péndulo invertido.

## Lección 8.3 --- Loop de control

Diseñar y posteriormente medir frecuencias para:

``` text
IMU
Encoders
Estimator
Controller
Motors
Telemetry
```

-   [ ] Definir períodos/deadlines.
-   [ ] Medir execution time.
-   [ ] Verificar jitter.
-   [ ] Confirmar que el loop cumple sus restricciones temporales.

## Lección 8.4 --- PID de equilibrio

-   [ ] Control P inicial.
-   [ ] PD.
-   [ ] PID si procede.
-   [ ] Saturación.
-   [ ] Fall detection.
-   [ ] Emergency motor shutdown.

## Lección 8.5 --- Balance

Primer objetivo funcional:

> El robot se mantiene vertical sin desplazarse significativamente.

-   [ ] Mantener equilibrio.
-   [ ] Medir oscilación.
-   [ ] Ajustar controlador.
-   [ ] Registrar resultados.

## Lección 8.6 --- Posición

Añadir encoders:

``` text
balance loop
     +
position loop
```

-   [ ] Mantener posición.
-   [ ] Recuperar desplazamientos.

## Lección 8.7 --- Movimiento

-   [ ] Avanzar.
-   [ ] Retroceder.
-   [ ] Girar.
-   [ ] Detenerse manteniendo equilibrio.

## Lección 8.8 --- LQR

Una vez funcional con PID:

``` text
ẋ = Ax + Bu
u = -Kx
```

-   [ ] Linearización.
-   [ ] State-space.
-   [ ] Controllability.
-   [ ] Selección Q/R.
-   [ ] Cálculo de K.
-   [ ] Implementación embedded.
-   [ ] Comparar PID vs LQR.

**Proyecto final:** primer robot autoequilibrado funcional.

------------------------------------------------------------------------

# Etapa 9 --- Robot personal tipo R2-D2

**Objetivo:** diseñar una plataforma robótica propia aplicando todo lo aprendido.

A partir de esta etapa los requisitos deben ser definidos por el desarrollador y no por un tutorial.

## Lección 9.1 --- Requirements

Definir formalmente qué debe hacer el robot.

Posibles capacidades:

-   [ ] Locomoción.
-   [ ] Balance.
-   [ ] Cabeza móvil.
-   [ ] Luces.
-   [ ] Sonidos.
-   [ ] Detección de obstáculos.
-   [ ] Control remoto.
-   [ ] Autonomía.

## Lección 9.2 --- Arquitectura

Estudiar una separación como:

``` text
High-level computer
Linux / perception / planning
          │
          │ UART / CAN / etc.
          ▼
Real-time controller
STM32 / control / motors / sensors
          │
          ▼
       Hardware
```

-   [ ] Definir responsabilidades del controlador real-time.
-   [ ] Definir responsabilidades del sistema high-level.
-   [ ] Diseñar interfaces entre subsistemas.

## Lección 9.3 --- Safety

-   [ ] Watchdog.
-   [ ] Brownout.
-   [ ] Motor shutdown.
-   [ ] Sensor timeout.
-   [ ] Fall detection.
-   [ ] Battery monitoring.
-   [ ] Fault state machine.
-   [ ] Reutilizar conceptos aprendidos en myECU.

## Lección 9.4 --- Comunicaciones

-   [ ] UART.
-   [ ] SPI.
-   [ ] I²C.
-   [ ] CAN.
-   [ ] Diseñar protocolo propio.
-   [ ] Checksums/CRC.
-   [ ] Timeouts.

## Lección 9.5 --- Integración

-   [ ] Bring-up por subsistemas.
-   [ ] Logging.
-   [ ] Telemetría.
-   [ ] Fault injection.
-   [ ] Pruebas de regresión.
-   [ ] Hardware-in-the-loop cuando corresponda.

## Lección 9.6 --- Robot V1

Integrar:

``` text
           ROBOT
             │
    ┌────────┼─────────┐
    │        │         │
Perception Control  Actuation
    │        │         │
    └────────┼─────────┘
             │
       embedded SW
             │
         electronics
             │
          mechanics
```

**Proyecto final:** primera versión de una plataforma robótica diseñada y desarrollada de forma propia.

------------------------------------------------------------------------

# Track paralelo --- C/C++ para sistemas embebidos

Los temas de C/C++ se introducirán cuando sean necesarios para resolver
problemas reales.

## Etapas 1--2

-   [ ] Registros.
-   [ ] Operaciones de bits.
-   [ ] `volatile`.
-   [ ] `const`.
-   [ ] Referencias.
-   [ ] Representación de datos.

## Etapa 3

-   [ ] Modelo de memoria.
-   [ ] Atomicidad.
-   [ ] ISR.
-   [ ] Concurrencia.
-   [ ] Secciones críticas.

## Etapa 4

-   [ ] Clases.
-   [ ] Interfaces.
-   [ ] Composición.
-   [ ] Separación de responsabilidades.

## Etapa 5

-   [ ] Ownership.
-   [ ] Stacks.
-   [ ] Concurrencia.
-   [ ] Recursos compartidos.

## Etapa 6

-   [ ] Drivers reutilizables.
-   [ ] HAL.
-   [ ] Arquitectura modular.
-   [ ] Controladores.

## Etapa 7

-   [ ] Floating point.
-   [ ] Precisión numérica.
-   [ ] Vectores y matrices.
-   [ ] Coste computacional.

## Etapa 8

-   [ ] Optimización.
-   [ ] Timing.
-   [ ] Determinismo.
-   [ ] Uso de memoria.
-   [ ] Instrumentación.

## Etapa 9

-   [ ] Arquitectura embedded completa.
-   [ ] Interfaces entre módulos.
-   [ ] Gestión de errores.
-   [ ] Diseño para pruebas.
-   [ ] Mantenibilidad.

------------------------------------------------------------------------

# Método de trabajo para cada periférico

Cada periférico o subsistema importante debe recorrer:

1.  [ ] **Teoría** --- comprender el principio de funcionamiento.
2.  [ ] **Datasheet / Reference Manual** --- consultar documentación del fabricante.
3.  [ ] **Cálculo en papel** --- determinar frecuencias, tiempos, escalas y límites.
4.  [ ] **Driver mínimo** --- implementar únicamente lo necesario.
5.  [ ] **Compilación** --- validar toolchain y linker.
6.  [ ] **GDB** --- observar el estado interno del firmware.
7.  [ ] **Medición física** --- comprobar la señal real.
8.  [ ] **Teoría vs realidad** --- comparar cálculos con mediciones.
9.  [ ] **Integración** --- incorporar el módulo al sistema completo.
10. [ ] **Documentación** --- registrar diseño, decisiones, resultados y limitaciones.

------------------------------------------------------------------------

# Secuencia general

``` text
myECU
  │
  ▼
[0] Cerrar myECU v1.0
  │
  ▼
[1] Servo PWM
  │
  ▼
[2] Encoder A/B
  │
  ▼
[3] Tiempo real / interrupciones / DMA
  │
  ▼
[4] Brazo robótico SG90
  │
  ▼
[5] Superloop + FreeRTOS
  │
  ▼
[6] Robot diferencial 2WD
  │
  ▼
[7] IMU + sensor fusion
  │
  ▼
[8] Péndulo invertido / robot autoequilibrado
  │
  ▼
[9] Robot personal tipo R2-D2
```

------------------------------------------------------------------------

# Uso previsto de las plataformas disponibles

## Arduino UNO R3

-   Experimentos rápidos.
-   Comparación Arduino framework vs implementación bare-metal.
-   Validaciones sencillas de módulos.

## STM32F103C8T6 Blue Pill

-   Fundamentos bare-metal.
-   GPIO.
-   ADC.
-   Timers.
-   Input Capture.
-   Encoder.
-   Interrupciones.
-   Continuación/cierre de myECU.

## NUCLEO-F411RE

-   Plataforma principal de aprendizaje embedded.
-   PWM.
-   Servos.
-   DMA.
-   Comunicaciones.
-   Arquitecturas de tiempo real.
-   FreeRTOS.
-   Primeros mecanismos robóticos.

## NUCLEO-F446RE

-   Robótica y control avanzado.
-   IMU.
-   Sensor fusion.
-   Floating point.
-   DSP.
-   Control PID/LQR.
-   Robot autoequilibrado.
-   Plataforma robótica final.

------------------------------------------------------------------------

# Próximo proyecto formal

Después de cerrar RPM y **myECU v1.0**:

## Proyecto 01 de Robótica --- NUCLEO-F411RE + SG90

**Objetivo:** generar y validar desde cero la señal de control de un servomotor SG90 utilizando:

-   NUCLEO-F411RE.
-   Timer STM32.
-   PWM.
-   C/C++.
-   Ubuntu 24.
-   Toolchain ARM.
-   GDB.
-   Analizador lógico.

**Regla:** no depender inicialmente de una biblioteca de servo que oculte la configuración del timer.

**Criterio de finalización:** comprender y poder demostrar físicamente toda la cadena:

``` text
Clock STM32
    ↓
Timer
    ↓
PWM / Output Compare
    ↓
pin físico
    ↓
señal medida
    ↓
SG90
    ↓
movimiento angular
```
