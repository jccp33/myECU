# Roadmap de alineación de myECU con Sistemas Automotrices C++ Mid Level

## 1. Objetivo

Este roadmap define las tareas necesarias para evolucionar **myECU**
desde su estado actual ---CORE portable, simulador Linux, diagnóstico,
máquinas de estados, logging y prototipo bare-metal STM32F103--- hacia
la arquitectura planteada en el documento **Sistemas Automotrices C++
Mid Level**, especialmente los hitos **Rango VIIA --- Gateway ECU v0.7**
y **Rango VIIB --- Control ECU v0.1**.

Principio rector:

> **No romper lo que ya funciona.** Las nuevas capacidades se integrarán
> alrededor del CORE actual siempre que sea posible. CAN, Linux,
> FreeRTOS, filesystem, SocketCAN y protocolos de diagnóstico no deben
> contaminar innecesariamente la lógica portable.

------------------------------------------------------------------------

## 2. Punto de partida de myECU

Actualmente myECU ya dispone de:

-   CORE portable en C++11.
-   `MessageManager`, `Message`, `Gateway`, `Control`, `FaultManager`,
    `FaultStateMachine` y `EcuStateMachine`.
-   Supervisión de rango y timeout.
-   Confirmación, recuperación y latching de faults.
-   Estados `INIT`, `SELF_TEST`, `OPERATIONAL`, `DEGRADED`,
    `SAFE_STATE`, `SHUTDOWN_REQ` y `SHUTDOWN`.
-   Configuración de once señales.
-   Simulación Linux manual y automática.
-   Inyección de fallos de rango y timeout.
-   Logging TXT de eventos y CSV de muestras.
-   Compilación con Make y CMake.
-   Suite de pruebas del CORE/pipeline.
-   Perfil embedded sin exceptions ni RTTI.
-   STM32F103C8T6 bare-metal.
-   ADC físico para TPS, temperatura NTC y MAP.
-   LEDs físicos asociados al estado global de la ECU.
-   Trabajo de adquisición RPM mediante TIM3 pendiente de integración
    definitiva.

El siguiente objetivo no es sustituir estas piezas, sino utilizarlas
como base para construir una arquitectura distribuida.

------------------------------------------------------------------------

# FASE 0 --- Cerrar y estabilizar la plataforma actual

**Objetivo:** establecer una baseline estable antes de introducir
comunicaciones automotrices.

## 0.1 Cerrar integración RPM

-   [ ] Revisar el estado actual de `rpm_input`.
-   [ ] Eliminar la renovación nominal de RPM que pueda ocultar timeout.
-   [ ] Resolver la inicialización del objeto/conversor RPM en el
    entorno bare-metal.
-   [ ] Validar captura física de período.
-   [ ] Validar conversión período → RPM.
-   [ ] Validar timeout cuando desaparecen los pulsos.
-   [ ] Integrar RPM en el pipeline completo:
    `RPM físico → Message → Gateway → FaultManager → Control → EcuState`.
-   [ ] Documentar evidencia y limitaciones.

## 0.2 Consolidar Logger

-   [ ] Mantener TXT para eventos/transiciones de la FSM.
-   [ ] Mantener CSV para muestras por ciclo.
-   [ ] Verificar creación robusta de `logs/txt` y `logs/csv`.
-   [ ] Registrar de forma inequívoca las señales que provocan
    `DEGRADED` y `SAFE_STATE`.
-   [ ] Añadir pruebas específicas del logger cuando la interfaz quede
    estable.
-   [ ] Mantener filesystem fuera del CORE.

## 0.3 Consolidar simulación

-   [ ] Mantener exclusión de faults `LATCHED` en la simulación
    automática normal.
-   [ ] Crear posteriormente escenarios explícitos para faults latched.
-   [ ] Añadir semilla configurable para simulaciones reproducibles.
-   [ ] Definir escenarios nominal, degraded, critical recoverable,
    latched y shutdown.
-   [ ] Conservar CSV como fuente de análisis temporal.

## 0.4 Baseline de pruebas

-   [ ] Ejecutar y dejar en verde la suite actual.
-   [ ] Ejecutar sanitizers en host.
-   [ ] Registrar resultados de compilación ARM.
-   [ ] Crear un tag/release de baseline antes de comenzar CAN.

**Criterio de salida:** myECU actual queda estable, reproducible y
documentado; las funciones existentes no dependen de la futura capa CAN.

------------------------------------------------------------------------

# FASE 1 --- Fundamentos CAN y laboratorio SocketCAN

**Esta será la primera fase nueva a trabajar después de cerrar la Fase
0.**

**Objetivo:** introducir CAN sin modificar todavía la semántica del
CORE.

## 1.1 Fundamentos

-   [ ] Estudiar estructura de una trama CAN.
-   [ ] Identificador estándar y extendido.
-   [ ] DLC.
-   [ ] Payload.
-   [ ] Arbitraje.
-   [ ] Error frames.
-   [ ] Diferenciar CAN clásico y CAN FD.

## 1.2 Laboratorio Linux

-   [ ] Instalar/verificar `can-utils`.
-   [ ] Crear interfaz virtual `vcan0`.
-   [ ] Levantar/bajar `vcan0`.
-   [ ] Enviar tramas con `cansend`.
-   [ ] Inspeccionar tráfico con `candump`.
-   [ ] Documentar comandos y capturas.

## 1.3 Abstracción C++ de CAN

Crear tipos fuera del CORE de dominio, por ejemplo:

``` text
CanFrame
CanInterface
SocketCanInterface
```

-   [ ] Representar ID, DLC y payload.
-   [ ] Implementar recepción.
-   [ ] Implementar transmisión.
-   [ ] Manejar errores sin introducir dependencias Linux en `core/`.
-   [ ] Crear pruebas para encode/decode básico.

## 1.4 Primer puente con myECU

Construir:

``` text
CAN Frame
   ↓
decoder
   ↓
valor de señal
   ↓
MessageManager
   ↓
Message
   ↓
Gateway
```

-   [ ] Elegir inicialmente una sola señal.
-   [ ] Recibirla desde `vcan0`.
-   [ ] Convertir payload a magnitud física.
-   [ ] Introducirla al pipeline actual.
-   [ ] Verificar que range/timeout siguen funcionando sin cambios en
    CORE.

**Criterio de salida:** una señal recibida por `vcan0` atraviesa el
pipeline actual de myECU.

------------------------------------------------------------------------

# FASE 2 --- Señales CAN y DBC

**Objetivo:** desacoplar la representación CAN de la política
diagnóstica de la ECU.

-   [ ] Estudiar start bit, length, endianness y signedness.
-   [ ] Implementar factor y offset.
-   [ ] Definir un DBC simplificado para señales de myECU.
-   [ ] Mapear señales actuales a mensajes CAN.
-   [ ] Implementar encoder.
-   [ ] Implementar decoder.
-   [ ] Mantener `SystemConfig` como política de diagnóstico/control.
-   [ ] No trasladar severity, confirmation, recovery o latching al DBC.
-   [ ] Crear pruebas con vectores conocidos.
-   [ ] Validar round-trip:
    `physical value → encode → CAN payload → decode → physical value`.

**Criterio de salida:** señales de myECU pueden codificarse y
reconstruirse de manera determinista desde tramas CAN.

------------------------------------------------------------------------

# FASE 3 --- Supervisión avanzada de comunicación CAN

**Objetivo:** incorporar los mecanismos de robustez pedidos para Gateway
ECU.

-   [ ] Validación de CAN ID.
-   [ ] Validación DLC.
-   [ ] Alive counter.
-   [ ] Rolling counter.
-   [ ] Checksum.
-   [ ] Timeout de mensajes CAN.
-   [ ] Periodicidad esperada.
-   [ ] Plausibilidad de señales.
-   [ ] Diseñar cómo estos errores se traducen a diagnóstico sin
    duplicar responsabilidades.
-   [ ] Integrarlos con `FaultManager` cuando corresponda.
-   [ ] Crear fault injection para cada mecanismo.

**Criterio de salida:** el Gateway detecta corrupción, pérdida,
repetición y ausencia de mensajes además de range/timeout de señal.

------------------------------------------------------------------------

# FASE 4 --- Separar Vehicle Simulator y Gateway ECU

**Objetivo:** dejar de alimentar directamente el pipeline y convertir
CAN en frontera entre procesos.

``` text
Vehicle Simulator
       │
       ▼
   CAN encoder
       │
       ▼
     vcan0
════════════════
       │
       ▼
  Gateway ECU
       │
       ▼
   DBC decoder
       │
       ▼
    myECU
```

-   [ ] Extraer un ejecutable `vehicle_simulator`.
-   [ ] Crear ejecutable `gateway_ecu`.
-   [ ] Comunicar ambos exclusivamente por CAN para las señales
    seleccionadas.
-   [ ] Observar tráfico con `candump`.
-   [ ] Mantener logging en el lado apropiado.
-   [ ] Crear pruebas de integración multiproceso.

**Criterio de salida:** simulador y Gateway son procesos independientes
comunicados por CAN virtual.

------------------------------------------------------------------------

# FASE 5 --- ISO-TP

**Objetivo:** añadir transporte de mensajes mayores que una trama CAN.

-   [ ] Estudiar Single Frame.
-   [ ] First Frame.
-   [ ] Consecutive Frame.
-   [ ] Flow Control.
-   [ ] Sequence number.
-   [ ] Timeouts.
-   [ ] Implementar o integrar ISO-TP sobre SocketCAN.
-   [ ] Probar segmentación y reensamblado.
-   [ ] Probar pérdida/orden incorrecto de frames.

**Criterio de salida:** Gateway puede intercambiar payloads diagnósticos
de longitud superior a una trama CAN.

------------------------------------------------------------------------

# FASE 6 --- UDS básico

**Objetivo:** exponer diagnóstico mediante servicios automotrices.

Implementar inicialmente:

-   [ ] `0x10` DiagnosticSessionControl.
-   [ ] `0x11` ECUReset.
-   [ ] `0x22` ReadDataByIdentifier.
-   [ ] Negative Response Codes.

Definir DIDs para:

-   [ ] versión de software;
-   [ ] estado global ECU;
-   [ ] temperatura;
-   [ ] voltaje;
-   [ ] velocidad;
-   [ ] información de health.

Arquitectura:

``` text
Tester
  ↓
UDS
  ↓
ISO-TP
  ↓
CAN
  ↓
Gateway ECU
  ↓
myECU state/data
```

**Criterio de salida:** un cliente diagnóstico puede consultar datos
reales del sistema mediante UDS.

------------------------------------------------------------------------

# FASE 7 --- DTC Manager

**Objetivo:** separar correctamente logging de diagnóstico persistente.

No convertir `Logger` en DTC manager.

``` text
FaultManager
   ├────────→ Logger
   │            ↓
   │         TXT/CSV
   │
   └────────→ DTC Manager
                ↓
               UDS
```

-   [ ] Diseñar estructura `DtcRecord`.
-   [ ] Mapear faults relevantes a DTCs.
-   [ ] Estado activo/histórico.
-   [ ] Snapshot/extended data simplificado.
-   [ ] Implementar `0x19 ReadDTCInformation`.
-   [ ] Implementar `0x14 ClearDiagnosticInformation`.
-   [ ] Evaluar persistencia para el target embedded.
-   [ ] Mantener Logger como herramienta de observabilidad/desarrollo.

**Criterio de salida:** faults pueden consultarse y borrarse mediante
servicios UDS sin depender de archivos TXT/CSV.

------------------------------------------------------------------------

# FASE 8 --- SOME/IP y publicación de señales

**Objetivo:** completar el Gateway ECU Linux del Rango VIIA.

-   [ ] Estudiar servicio, instancia, método, evento y campo.
-   [ ] Configurar `vsomeip`.
-   [ ] Publicar disponibilidad del Gateway.
-   [ ] Exponer señales seleccionadas.
-   [ ] Publicar eventos.
-   [ ] Implementar cliente.
-   [ ] Manejar desconexión/reconexión.
-   [ ] Validar tráfico con Wireshark.

**Criterio de salida:** Gateway recibe información vehicular por CAN y
la expone a un cliente mediante SOME/IP.

------------------------------------------------------------------------

# HITO A --- Gateway ECU v0.7

Al finalizar las fases 1--8:

``` text
Vehicle Simulator
       │
      CAN
       ▼
┌──────────────────────────┐
│       Gateway ECU        │
│          Linux           │
│                          │
│ SocketCAN                │
│ CAN / CAN FD             │
│ DBC                      │
│ counters/checksum        │
│ ISO-TP                   │
│ UDS                      │
│ DTC                      │
│ SOME/IP                  │
└──────────────────────────┘
```

-   [ ] Documentar arquitectura.
-   [ ] Documentar protocolos.
-   [ ] Documentar pruebas.
-   [ ] Generar release reproducible.

------------------------------------------------------------------------

# FASE 9 --- CAN físico entre Linux y STM32

**Objetivo:** preparar la separación real Gateway ECU ↔ Control ECU.

-   [ ] Seleccionar transceptor/controlador CAN apropiado para el
    hardware.
-   [ ] Implementar driver CAN del STM32.
-   [ ] Mantener driver en `platform/stm32`.
-   [ ] Crear adaptación en `app/stm32`.
-   [ ] No introducir registros CAN dentro de `core/`.
-   [ ] Probar transmisión STM32 → Linux.
-   [ ] Probar recepción Linux → STM32.
-   [ ] Integrar señales CAN con `MessageManager`.

**Criterio de salida:** Linux Gateway y STM32 intercambian señales
reales mediante CAN.

------------------------------------------------------------------------

# FASE 10 --- Introducción de FreeRTOS

**Objetivo:** transformar la Control ECU bare-metal en un nodo de tiempo
real sin contaminar el CORE.

Posible distribución:

``` text
AcquisitionTask
CanRxTask
ControlTask
DiagnosticTask
SupervisorTask
```

-   [ ] Integrar FreeRTOS.
-   [ ] Definir períodos.
-   [ ] Definir prioridades.
-   [ ] Definir deadlines.
-   [ ] Medir ejecución.
-   [ ] Detectar deadline misses.
-   [ ] Introducir queues.
-   [ ] Introducir semáforos donde sean necesarios.
-   [ ] Introducir mutex solo para recursos compartidos.
-   [ ] Estudiar priority inversion.
-   [ ] Validar priority inheritance.
-   [ ] Dimensionar stack por tarea.
-   [ ] Evitar heap dinámico durante operación crítica.

Principio:

> FreeRTOS determina **cuándo** se ejecuta la lógica; el CORE determina
> **qué significa** el estado de la ECU.

**Criterio de salida:** el mismo CORE funciona bajo un scheduler RTOS
con comportamiento temporal medible.

------------------------------------------------------------------------

# FASE 11 --- Health monitoring, heartbeat y watchdog

-   [ ] Crear `HealthMonitor`.
-   [ ] Supervisar tareas críticas.
-   [ ] Heartbeats.
-   [ ] Watchdog.
-   [ ] Detectar tarea bloqueada.
-   [ ] Detectar deadline miss.
-   [ ] Propagar errores relevantes hacia el modelo de faults.
-   [ ] Definir transición a `DEGRADED`.
-   [ ] Definir transición a `SAFE_STATE`.
-   [ ] Probar recuperación cuando sea permitida.
-   [ ] Probar faults no recuperables.

**Criterio de salida:** la ECU supervisa no solo sensores, sino también
su propia capacidad de ejecución.

------------------------------------------------------------------------

# FASE 12 --- Functional Safety educativa

**Objetivo:** relacionar la arquitectura existente con conceptos de ISO
26262 sin afirmar cumplimiento normativo.\*\*

-   [ ] Estudiar ASIL A--D.
-   [ ] Identificar funciones safety-related del prototipo.
-   [ ] Definir safe states.
-   [ ] Identificar single points of failure.
-   [ ] Analizar fault detection.
-   [ ] Analizar fault tolerance.
-   [ ] Documentar supuestos.
-   [ ] Documentar limitaciones.
-   [ ] Relacionar `DEGRADED` y `SAFE_STATE` con los mecanismos de
    supervisión.

**Criterio de salida:** existe una justificación técnica documentada de
las políticas fail-safe del prototipo.

------------------------------------------------------------------------

# FASE 13 --- Cybersecurity básica

-   [ ] Identificar superficies de ataque.
-   [ ] Gateway Linux.
-   [ ] CAN.
-   [ ] UDS.
-   [ ] SOME/IP.
-   [ ] Control ECU.
-   [ ] Realizar TARA educativa.
-   [ ] Estudiar autenticidad, integridad, confidencialidad y
    disponibilidad.
-   [ ] Freshness values.
-   [ ] Replay protection.
-   [ ] Restringir servicios diagnósticos sensibles.

**Criterio de salida:** amenazas principales y mitigaciones quedan
documentadas y probadas donde sea razonable.

------------------------------------------------------------------------

# FASE 14 --- MISRA C++ 2023 educativo y análisis estático

-   [ ] Definir subconjunto de reglas aplicable al proyecto.
-   [ ] Ejecutar `clang-tidy`.
-   [ ] Identificar undefined behavior.
-   [ ] Revisar conversiones.
-   [ ] Revisar loops acotados.
-   [ ] Revisar inicialización.
-   [ ] Revisar uso de memoria dinámica.
-   [ ] Revisar excepciones.
-   [ ] Documentar deviations.
-   [ ] No refactorizar masivamente: corregir por módulos y con pruebas.

**Criterio de salida:** existe reporte reproducible de análisis estático
y deviations justificadas.

------------------------------------------------------------------------

# FASE 15 --- Coverage, MC/DC, SIL y fault injection

-   [ ] Medir line coverage.
-   [ ] Medir branch coverage.
-   [ ] Identificar decisiones críticas.
-   [ ] Diseñar casos MC/DC seleccionados.
-   [ ] Aplicar MC/DC especialmente a decisiones de fault/FSM.
-   [ ] Automatizar fault injection.
-   [ ] Crear SIL.
-   [ ] Ejecutar Control ECU en QEMU Cortex-M cuando sea compatible.
-   [ ] Comparar comportamiento host/QEMU/hardware.
-   [ ] Crear pruebas de integración Gateway ECU ↔ Control ECU.

**Criterio de salida:** las decisiones críticas cuentan con evidencia de
cobertura y los principales escenarios de fallo son reproducibles.

------------------------------------------------------------------------

# FASE 16 --- CI/CD y Release Engineering

-   [ ] Crear GitHub Actions.
-   [ ] Build Linux.
-   [ ] Build CORE.
-   [ ] Ejecutar tests.
-   [ ] Sanitizers.
-   [ ] Static analysis.
-   [ ] Coverage.
-   [ ] Cross-compilation ARM.
-   [ ] Definir gates.
-   [ ] Versionado.
-   [ ] Changelog.
-   [ ] Artefactos binarios.
-   [ ] Reporte de coverage.
-   [ ] Reporte de análisis estático.
-   [ ] Reporte de deviations.
-   [ ] Crear release reproducible.

------------------------------------------------------------------------

# HITO B --- Control ECU v0.1

Arquitectura objetivo:

``` text
                    CAN
                     │
          ┌──────────▼──────────┐
          │     Control ECU     │
          │      Cortex-M       │
          │                     │
          │      FreeRTOS       │
          │                     │
          │ AcquisitionTask     │
          │ CanRx/TxTask        │
          │ ControlTask         │
          │ DiagnosticTask      │
          │ SupervisorTask      │
          │                     │
          │ myECU CORE          │
          │ FaultManager        │
          │ EcuStateMachine     │
          │ HealthMonitor       │
          │ Watchdog            │
          └─────────────────────┘
```

La arquitectura distribuida final queda:

``` text
┌─────────────────────────┐
│       Gateway ECU       │
│          Linux          │
│                         │
│ CAN / DBC               │
│ ISO-TP / UDS            │
│ DTC                     │
│ SOME/IP                 │
└────────────┬────────────┘
             │
             │ CAN
             │
┌────────────▼────────────┐
│       Control ECU       │
│        Cortex-M         │
│                         │
│ FreeRTOS                │
│ Acquisition             │
│ Control                 │
│ Fault Management        │
│ Health / Watchdog       │
│ Safe State              │
└─────────────────────────┘
```

------------------------------------------------------------------------

# Orden de ejecución recomendado

``` text
FASE 0  → estabilizar myECU actual
   ↓
FASE 1  → CAN + SocketCAN + vcan0
   ↓
FASE 2  → DBC
   ↓
FASE 3  → validaciones CAN
   ↓
FASE 4  → separar simulador/Gateway
   ↓
FASE 5  → ISO-TP
   ↓
FASE 6  → UDS
   ↓
FASE 7  → DTC
   ↓
FASE 8  → SOME/IP
   ↓
HITO A   → Gateway ECU v0.7
   ↓
FASE 9  → CAN físico STM32/Linux
   ↓
FASE 10 → FreeRTOS
   ↓
FASE 11 → Health/Watchdog
   ↓
FASE 12 → Safety
   ↓
FASE 13 → Cybersecurity
   ↓
FASE 14 → MISRA/análisis estático
   ↓
FASE 15 → Coverage/MC-DC/SIL
   ↓
FASE 16 → CI/CD + Release
   ↓
HITO B   → Control ECU v0.1
```

------------------------------------------------------------------------

# Primera tarea activa

## FASE 0.1 --- Cerrar integración RPM

Esta será la **primera tarea a trabajar** antes de introducir CAN.

El objetivo inmediato es eliminar la discrepancia actual entre la prueba
aislada de RPM y su integración en `control_test`, manteniendo intacto
el pipeline que ya funciona.

Secuencia de trabajo:

1.  Revisar de nuevo el código remoto vigente relacionado con RPM.
2.  Identificar exactamente cómo se inicializa `RpmSensor`.
3.  Corregir primero la inicialización bare-metal si continúa
    dependiendo de constructores globales no ejecutados.
4.  Instrumentar captura aceptada/rechazada sin modificar `FaultManager`
    ni `Control`.
5.  Validar físicamente TIM3/PA6.
6.  Integrar la muestra RPM real en `MessageManager`.
7.  Eliminar únicamente entonces la renovación nominal de RPM.
8.  Comprobar:
    -   pulsos → RPM válida;
    -   ausencia de pulsos → `TIMEOUT`;
    -   RPM fuera de rango → fault;
    -   recuperación → estado esperado.
9.  Documentar el resultado.
10. Ejecutar regresión del resto de señales y FSM.

**Regla de trabajo:** una modificación verificable a la vez; no
refactorizar componentes no relacionados.

------------------------------------------------------------------------

## Definición de terminado de la Fase 0.1

La tarea se considera cerrada cuando pueda demostrarse físicamente:

``` text
Encoder / generador de pulsos
        ↓
PA6 / TIM3_CH1
        ↓
rpm_input
        ↓
RpmSensor
        ↓
MessageManager
        ↓
Message RPM
        ↓
Gateway
        ↓
SignalStatus
        ↓
FaultManager
        ↓
Control
        ↓
EcuState
```

sin una actualización nominal de RPM que oculte la ausencia de pulsos.
