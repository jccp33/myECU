# myECU — CORE Architecture Purge Checklist

## Objetivo

Garantizar que el CORE de **myECU** cumpla con los siguientes principios arquitectónicos:

- No exista duplicidad injustificada de funcionalidades.
- No exista duplicidad injustificada de datos.
- No existan múltiples fuentes autoritativas para una misma configuración o política.
- Cada componente tenga una responsabilidad única, explícita y verificable.
- Las dependencias entre componentes sean explícitas, justificadas y unidireccionales.
- No existan rutas paralelas que implementen la misma decisión diagnóstica.
- Los datos derivados no puedan convertirse accidentalmente en nuevas fuentes de verdad.
- El CORE sea completamente independiente de Linux, Windows, STM32, AVR u otra plataforma.
- El CORE no conozca GPIO, ADC, HAL, drivers, consola, teclado, terminal ni mecanismos concretos de adquisición.
- El CORE pueda compilarse sin modificar su código fuente en los toolchains objetivo.
- El comportamiento temporal del CORE sea determinista y explícito.
- La simulación por consola y STM32 utilicen el mismo pipeline funcional del CORE.
- Makefile y CMake describan exactamente el mismo CORE.
- La estructura física del repositorio refleje la arquitectura lógica.
- Los tests demuestren las propiedades arquitectónicas anteriores.
- La documentación describa la arquitectura realmente implementada.

---

# 1. Eliminar residuos de arquitecturas anteriores

- [x] Auditar y eliminar `EvaluationType` si ningún componente vigente lo necesita.
- [x] Auditar y eliminar `RuleValidationError` si no tiene consumidor.
- [x] Auditar y eliminar `SignalError` si `SignalStatus` representa oficialmente el estado de validación.
- [x] Auditar y eliminar `FaultType` si no existe un requisito funcional que lo utilice.
- [x] Eliminar `fault_types.hpp` cuando todos sus consumidores utilicen directamente `data_types.hpp`.
- [x] Confirmar que `FaultConditionEvaluator` no exista en código productivo.
- [x] Confirmar que `FaultConditionEvaluator` no exista en Makefile.
- [x] Confirmar que `FaultConditionEvaluator` no exista en CMake.
- [x] Buscar cualquier implementación residual del antiguo modelo `RANGE/TIMEOUT`.
- [x] Identificar referencias obsoletas en README y documentación.
- [x] Actualizar `docs/ARCHITECTURE.md` para eliminar la arquitectura diagnóstica obsoleta.
- [ ] Realizar al final de la purga una búsqueda global adicional de símbolos, archivos y documentación correspondientes a arquitecturas eliminadas.

## Criterio de cierre

Ningún tipo, clase, archivo, dependencia o documentación vigente correspondiente a una arquitectura eliminada permanece dentro del CORE productivo.

---

# 2. Garantizar una única fuente de verdad para cada dato

## Identidades

- [x] Auditar `messageId` frente a `SignalId.id`.
- [x] Eliminar `messageId` al representar la misma identidad que `SignalId`.
- [x] Auditar `SensorId` frente a `SignalId`.
- [x] Separar identidad funcional del CORE de identidad utilizada por el simulador.
- [x] Eliminar `SensorId` de `Message`.
- [x] Eliminar `SensorId` de `MessageManager`.
- [x] Eliminar `SensorId` de `InitValues`.
- [x] Eliminar `SensorId` de los tipos del CORE.
- [x] Mantener `SensorId` exclusivamente como concepto del simulador mientras sea necesario.
- [x] Documentar claramente la responsabilidad de `SensorId`.
- [x] Documentar claramente la responsabilidad de `SignalId`.
- [ ] Revisar si el mapeo del simulador basado únicamente en `SignalId.id` es suficiente o ignora información relevante de la identidad completa de `SignalId`.
- [x] Confirmar que ningún componente productivo mantenga una segunda identidad equivalente a `SignalId`.

## Roles funcionales

- [x] Eliminar la dependencia de `Control` respecto a una identidad concreta como `SensorId::BRAKE`.
- [x] Representar explícitamente el rol de solicitud de apagado.
- [x] Representar explícitamente el rol de permiso de apagado.
- [ ] Auditar si `isShutdownRequest` e `isShutdownPermission` son la representación definitiva adecuada o si constituyen metadata funcional que debe modelarse de otra manera.
- [x] Confirmar que ningún otro comportamiento funcional dependa accidentalmente de IDs concretos.

## Configuración diagnóstica

- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `minValue`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `maxValue`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `timeoutMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `confirmationTimeMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `recoveryTimeMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `FaultSeverity`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `FaultLatching`.
- [ ] Buscar nuevamente valores diagnósticos hardcodeados fuera de `SystemConfig`.
- [ ] Buscar IDs de señales hardcodeados que representen política y no simple adaptación/mapeo.
- [ ] Confirmar que modificar una política en `SystemConfig` altere efectivamente el comportamiento correspondiente sin modificar ningún segundo archivo.

## Datos derivados

- [x] Auditar `Message::severity`.
- [x] Eliminar `Message::severity` al no existir un consumidor operativo que necesite esa copia.
- [x] Confirmar que `EvaluationRule` sea una representación derivada de `SystemConfig`.
- [x] Impedir mediante validación que `EvaluationRule` introduzca silenciosamente una configuración divergente.
- [x] Fortalecer `validateEvaluationRuleSet()` para comparar la política derivada con `SystemConfig`.
- [x] Auditar todos los demás campos copiados desde configuración hacia objetos runtime.
- [x] Clasificar cada copia como configuración autoritativa, snapshot, estado runtime o dato derivado.
- [x] Confirmar que ninguna copia derivada pueda modificarse independientemente y convertirse en una segunda fuente de verdad.

## Configuración potencialmente muerta

- [x] Definir formalmente la semántica de `SystemConfig::maxInvalidSignals`.
- [x] Determinar que `maxInvalidSignals` no forma parte del diseño funcional actual.
- [x] Eliminar `maxInvalidSignals` al no existir un requisito funcional que lo justifique.
- [ ] Auditar todos los demás campos de `SystemConfig` para confirmar que tengan consumidores reales.
- [ ] Eliminar cualquier configuración sin efecto funcional o documentar claramente su propósito futuro si existe un requisito real.

## Criterio de cierre

Modificar cualquier política de una señal requiere modificar un único lugar.

```text
SystemConfig
    |
    +--> Message --> Gateway
    |
    +--> EvaluationRule
             |
             +--> FaultManager
```

Las copias runtime son derivadas y no constituyen configuraciones independientes.

---

# 3. Garantizar responsabilidad única de los componentes

## Message

- [x] Definir formalmente la responsabilidad de `Message`.
- [x] Confirmar que `Message` represente datos/runtime metadata de una señal y no ejecute política diagnóstica.
- [x] Confirmar que `Message` no duplique información innecesaria de configuración.
- [x] Auditar todos sus campos y métodos.
- [x] Eliminar campos o métodos que pertenezcan a otra responsabilidad.

## MessageManager

- [x] Definir formalmente la responsabilidad de `MessageManager`.
- [x] Confirmar que no implemente validación diagnóstica.
- [x] Confirmar que no implemente lógica de estado de ECU.
- [x] Confirmar que no mantenga configuración independiente.
- [x] Revisar si su responsabilidad justifica la existencia de la clase o si existe solapamiento con otro componente.

## Gateway

- [x] Definir formalmente `Gateway` como autoridad para establecer `SignalStatus`.
- [x] Confirmar que realice las comparaciones de rango exactamente una vez.
- [x] Confirmar que realice la evaluación de timeout exactamente una vez.
- [x] Confirmar que ningún otro componente vuelva a comparar `minValue/maxValue`.
- [x] Confirmar que ningún otro componente vuelva a decidir timeout.
- [x] Confirmar que `Gateway` no decida severidad.
- [x] Confirmar que `Gateway` no gestione confirmación temporal de faults.
- [x] Confirmar que `Gateway` no decida directamente el estado global de ECU.

## Control

- [x] Definir formalmente la responsabilidad de `Control`.
- [x] Confirmar que interprete `SignalStatus`, criticidad/política y condiciones del sistema.
- [x] Confirmar que `Control` no vuelva a evaluar rangos.
- [x] Confirmar que `Control` no vuelva a evaluar timeout.
- [x] Confirmar que `Control` no duplique lógica de `FaultManager`.
- [x] Confirmar que `Control` no duplique lógica de `EcuStateMachine`.
- [x] Confirmar que las entradas que produce para la FSM sean explícitas y suficientes.

## EvaluationRule

- [x] Definir formalmente la responsabilidad de `EvaluationRule`.
- [x] Confirmar que solo represente política temporal/diagnóstica derivada.
- [x] Confirmar que no evalúe rango ni timeout.
- [x] Confirmar que no mantenga estado runtime de fault.
- [x] Confirmar que no se convierta en una configuración autoritativa paralela.

## FaultStateMachine

- [x] Definir formalmente la responsabilidad de `FaultStateMachine`.
- [x] Confirmar que reciba una condición booleana ya evaluada.
- [x] Confirmar que únicamente gestione confirmación, recuperación y latching.
- [x] Confirmar que no conozca valores físicos de señales.
- [x] Confirmar que no conozca rangos ni timeouts.
- [x] Confirmar que no determine el estado global de la ECU.

## FaultManager

- [x] Definir formalmente la responsabilidad de `FaultManager`.
- [ ] Confirmar la relación uno-a-uno entre señal diagnóstica y `FaultRecord`.
- [x] Confirmar que `OUT_OF_RANGE` y `TIMEOUT` sean causas/condiciones y no faults paralelos independientes para la misma señal.
- [x] Confirmar que no vuelva a evaluar min/max.
- [x] Confirmar que no vuelva a evaluar timeout.
- [x] Confirmar que no mantenga una segunda configuración diagnóstica.
- [x] Confirmar que la severidad utilizada provenga de `EvaluationRule/SystemConfig`.
- [x] Revisar cómo se construye y actualiza `FaultSummary`.

## EcuStateMachine

- [x] Definir formalmente la responsabilidad de `EcuStateMachine`.
- [x] Confirmar que opere únicamente sobre entradas de dominio.
- [x] Confirmar que no conozca señales físicas concretas.
- [x] Confirmar que no conozca sensores, ADC, GPIO o plataforma.
- [x] Confirmar que no reimplemente decisiones que correspondan a `Control` o `FaultManager`.

## FaultConfiguration

- [x] Definir formalmente la responsabilidad de `FaultConfiguration`.
- [x] Confirmar que solamente derive y valide reglas desde `SystemConfig`.
- [x] Confirmar que no contenga política diagnóstica hardcodeada.
- [x] Confirmar que no sea una segunda fuente de configuración.

## DiagnosticStatus

- [x] Definir formalmente la responsabilidad de `DiagnosticStatus`.
- [x] Auditar `DiagnosticStatus::INVALID_SIGNAL_SAMPLE`.
- [x] Determinar si `INVALID_SIGNAL_SAMPLE` continúa teniendo un consumidor y significado válidos.
- [x] Eliminar estados diagnósticos muertos o redundantes.

## Clases originales del diseño

Las siguientes clases forman parte deliberada del diseño original y **no deben eliminarse únicamente como consecuencia de una refactorización posterior**:

- `Message`
- `MessageManager`
- `Gateway`
- `Control`

- [ ] Preservar su identidad conceptual.
- [ ] Adaptar las abstracciones añadidas posteriormente alrededor de estas responsabilidades.
- [ ] Refactorizar campos o responsabilidades internas únicamente cuando exista duplicidad o violación arquitectónica demostrable.
- [ ] Confirmar que ninguna abstracción añadida haya sustituido accidentalmente la responsabilidad original de estas clases.

## Criterio de cierre

Cada clase puede describirse con una responsabilidad principal inequívoca y ninguna decisión funcional se implementa en dos componentes diferentes.

---

# 4. Resolver definitivamente SignalSample y SignalStore

## SignalSample

- [ ] Definir formalmente qué representa `SignalSample`.
- [ ] Determinar si representa adquisición, transporte, dominio o infraestructura.
- [ ] Comparar sus datos con `Message`.
- [ ] Identificar cualquier duplicidad entre `SignalSample` y `Message`.
- [ ] Determinar si ambas abstracciones son necesarias.
- [ ] Si ambas permanecen, documentar la frontera exacta entre ellas.
- [ ] Confirmar que `SignalSample` no constituya una segunda representación diagnóstica de la misma señal.

## SignalStore

- [ ] Definir formalmente qué representa `SignalStore`.
- [ ] Determinar si pertenece a adquisición, aplicación, infraestructura o CORE.
- [ ] Confirmar que su función de almacenamiento no duplique `MessageManager`.
- [ ] Confirmar que no implemente política diagnóstica.
- [ ] Confirmar que no sea una segunda fuente de estado de fault.
- [ ] Revisar `upsert()`, `find()`, `get()`, `clear()` y demás API.
- [ ] Revisar capacidad, almacenamiento y comportamiento cuando se alcanza el límite.
- [ ] Revisar comportamiento ante `SignalId` duplicados.
- [ ] Revisar comportamiento temporal de muestras reemplazadas.
- [ ] Decidir ubicación física definitiva de `signal_sample.hpp`.
- [ ] Decidir ubicación física definitiva de `signal_store.hpp/.cpp`.

## Pipeline

- [ ] Comparar explícitamente el pipeline de consola con el pipeline STM32.
- [ ] Detectar si actualmente existen dos pipelines diagnósticos.
- [ ] Impedir que STM32 use `SignalStore -> FaultManager` si eso evita `Message -> Gateway -> Control` sin una justificación arquitectónica.
- [ ] Diseñar una única frontera entre adquisición y CORE.
- [ ] Confirmar que consola y STM32 alimenten el mismo contrato del CORE.

## Criterio de cierre

Existe una sola ruta diagnóstica oficial.

La diferencia entre plataformas termina en la frontera de adquisición.

---

# 5. Auditar acoplamiento y dirección de dependencias

- [x] Construir el grafo real de dependencias entre headers y clases del CORE.
- [x] Confirmar que las dependencias sigan una dirección arquitectónica justificable.
- [x] Detectar dependencias circulares.
- [x] Detectar includes transitivos utilizados accidentalmente.
- [x] Confirmar que cada header incluya directamente lo que necesita.
- [x] Confirmar que los headers públicos no requieran orden especial de inclusión.
- [x] Revisar dependencias entre `MessageManager` y `Message`.
- [x] Revisar dependencias entre `Gateway` y `Message`.
- [x] Revisar dependencias entre `Control`, `FaultManager` y `EcuStateMachine`.
- [x] Revisar dependencias entre `FaultManager`, `FaultStateMachine` y `EvaluationRule`.
- [x] Revisar dependencias entre `FaultConfiguration`, `SystemConfig` y `EvaluationRule`.
- [x] Eliminar dependencias cuya única razón sea conveniencia y no responsabilidad.
- [x] Evitar que componentes de bajo nivel dependan de decisiones de aplicación.
- [ ] Confirmar que `platform/` no dependa de semántica de dominio innecesariamente.
- [x] Confirmar que CORE no dependa de aplicación ni plataforma.

## Criterio de cierre

El grafo de dependencias es acíclico o cualquier excepción está explícitamente justificada, y las dependencias apuntan hacia abstracciones estables del dominio.

---

# 6. Demostrar independencia total de plataforma

## CORE

- [x] Buscar includes específicos de Linux dentro de `core/`.
- [x] Buscar includes específicos de Windows dentro de `core/`.
- [x] Buscar includes específicos de STM32 dentro de `core/`.
- [x] Buscar includes específicos de AVR dentro de `core/`.
- [x] Buscar referencias a HAL dentro de `core/`.
- [x] Buscar referencias a GPIO dentro de `core/`.
- [x] Buscar referencias a ADC dentro de `core/`.
- [x] Buscar referencias a UART dentro de `core/`.
- [x] Buscar referencias a consola/terminal dentro de `core/`.
- [x] Buscar llamadas de sistema dentro de `core/`.
- [x] Buscar dependencias de tiempo del sistema operativo.
- [x] Confirmar que el tiempo entre al CORE como dato explícito.
- [x] Confirmar que CORE no acceda directamente a hardware.
- [x] Confirmar que CORE no realice I/O de plataforma.

## Estructura física

- [x] Crear `core/include/`.
- [x] Crear `core/src/`.
- [x] Mover los headers inequívocamente pertenecientes al CORE a `core/include/`.
- [x] Mover las implementaciones inequívocamente pertenecientes al CORE a `core/src/`.
- [x] Mantener `config.hpp/.cpp` fuera del CORE.
- [x] Mantener `sensor_simulation` fuera del CORE.
- [x] Mantener `simulations.cpp` fuera del CORE.
- [x] Mantener `linux_platform.cpp` fuera del CORE.
- [x] Mantener `SignalSample/SignalStore` fuera del CORE hasta completar su auditoría.
- [ ] Revisar al final que ningún archivo haya quedado físicamente en una capa incorrecta.

## Criterio de cierre

La carpeta `core/` puede copiarse a otro proyecto/plataforma y compilarse sin necesitar código Linux, Windows, AVR o STM32.

---

# 7. Auditar portabilidad C++ y perfil embedded

- [x] Confirmar estándar mínimo de C++ requerido por el CORE.
- [x] Confirmar que todo el CORE compile como C++11 si C++11 continúa siendo el requisito.
- [x] Compilar todos los headers públicos aisladamente.
- [x] Compilar el CORE con `ECU_CORE_EMBEDDED_PROFILE=ON`.
- [x] Revisar compatibilidad con `-ffreestanding`.
- [x] Revisar compatibilidad con `-fno-exceptions`.
- [x] Revisar compatibilidad con `-fno-rtti`.
- [x] Revisar compatibilidad con `-fno-threadsafe-statics`.
- [x] Revisar compatibilidad con `-fno-use-cxa-atexit`.
- [x] Buscar uso de excepciones.
- [x] Buscar RTTI (`dynamic_cast`, `typeid`).
- [x] Buscar asignación dinámica (`new`, `delete`, `malloc`, `free`).
- [x] Buscar contenedores o APIs que puedan introducir asignación dinámica.
- [x] Auditar el uso de STL dentro del CORE.
- [x] Confirmar que cualquier STL utilizada tenga coste y comportamiento apropiados para embedded.
- [x] Auditar objetos estáticos/globales y su inicialización.
- [x] Auditar constructores globales.
- [x] Auditar destrucción global.
- [ ] Revisar tamaños de tipos dependientes de arquitectura.
- [ ] Revisar conversiones signed/unsigned.
- [ ] Revisar conversiones narrowing.
- [ ] Revisar posibles overflows/underflows.
- [ ] Revisar alineación y padding cuando sean relevantes.
- [x] Confirmar que no se dependa accidentalmente del tamaño de puntero del host.
- [x] Compilar el CORE con un toolchain ARM objetivo sin modificar fuentes.
- [x] Documentar cualquier dependencia inevitable de la biblioteca estándar.

## Criterio de cierre

El mismo código fuente de `core/` compila en host y ARM con el perfil requerido, sin adaptaciones condicionales de plataforma dentro del dominio.

---

# 8. Auditar comportamiento temporal y determinismo

## Timestamp

- [ ] Identificar todos los lugares donde se genera, copia o consume `TimestampMs`.
- [ ] Definir formalmente las unidades de `TimestampMs`.
- [ ] Definir formalmente su origen y semántica.
- [ ] Confirmar que un ciclo de procesamiento utilice un único `now`.
- [ ] Evitar lecturas independientes del reloj durante una misma evaluación lógica.
- [ ] Confirmar que simulador y STM32 proporcionen el tiempo mediante el mismo contrato.

## Gateway y timeout

- [ ] Auditar la resta unsigned utilizada para calcular edad de una muestra.
- [ ] Definir comportamiento cuando `currentTimeMs < timestamp`.
- [ ] Determinar la política frente a regresión del reloj.
- [ ] Determinar la política frente a wraparound del contador.
- [ ] Confirmar que timeout sea determinista en esos casos.
- [ ] Confirmar que no exista otra implementación de timeout fuera de `Gateway`.

## FaultStateMachine

- [ ] Verificar semántica exacta de `confirmationTimeMs`.
- [ ] Verificar semántica exacta de `recoveryTimeMs`.
- [ ] Verificar comportamiento con tiempo igual al umbral.
- [ ] Verificar comportamiento con saltos grandes de tiempo.
- [ ] Verificar comportamiento con timestamp repetido.
- [ ] Verificar comportamiento con regresión temporal.
- [ ] Verificar comportamiento de faults latched.
- [ ] Verificar comportamiento de recuperación de faults no latched.

## Determinismo

- [ ] Confirmar ausencia de dependencias de orden accidental entre señales.
- [ ] Confirmar que procesar el mismo conjunto de entradas y timestamps produzca el mismo resultado.
- [ ] Confirmar que ningún comportamiento del CORE dependa de scheduling del host.
- [ ] Confirmar que ningún comportamiento del CORE dependa de I/O asíncrono no representado explícitamente.

## Criterio de cierre

El comportamiento temporal puede explicarse completamente a partir de entradas y timestamps explícitos.

---

# 9. Auditar comparaciones numéricas y semántica de valores

- [ ] Localizar todas las comparaciones exactas de `float`.
- [ ] Revisar específicamente la comparación exacta utilizada por la lógica de permiso/solicitud de apagado.
- [ ] Determinar cuáles comparaciones exactas son válidas por tratarse de valores discretos codificados.
- [ ] Sustituir por umbral/tolerancia las comparaciones donde el valor represente una magnitud analógica.
- [ ] Documentar la semántica de valores booleanos representados como `float`, si permanecen.
- [ ] Determinar si esos valores deberían representarse mediante un tipo más apropiado.
- [ ] Revisar NaN/Inf si pueden entrar desde alguna frontera.
- [ ] Revisar límites inclusivos/exclusivos de min/max.
- [ ] Confirmar comportamiento en exactamente `minValue`.
- [ ] Confirmar comportamiento en exactamente `maxValue`.

## Criterio de cierre

Toda comparación numérica tiene una semántica explícita apropiada para el tipo de señal representada.

---

# 10. Auditar invariantes y validación de configuración

## SystemConfig

- [ ] Validar `sensorCount` frente a capacidad real.
- [ ] Validar `SignalId` duplicados.
- [ ] Validar rangos incoherentes (`minValue > maxValue`).
- [ ] Validar timeouts inválidos si existe tal restricción.
- [ ] Validar políticas temporales inválidas.
- [ ] Validar combinaciones de severidad/latching si existen restricciones.
- [ ] Validar roles funcionales incompatibles o duplicados si corresponde.

## EvaluationRuleSet

- [x] Verificar que cada regla corresponda a una señal existente.
- [x] Comparar política de cada regla con `SystemConfig`.
- [x] Detectar reglas duplicadas durante la validación del conjunto.
- [x] Revisar construcción directa de `EvaluationRule` fuera de `FaultConfiguration`.
- [ ] Decidir si la API debe restringirse adicionalmente para impedir reglas arbitrarias en producción.

## FaultManager

- [ ] Auditar qué ocurre si recibe directamente reglas con `SignalId` duplicados.
- [ ] Decidir si `FaultManager` debe rechazar duplicados independientemente de la validación externa.
- [x] Confirmar que capacidad y número de reglas tengan invariantes explícitos.
- [x] Confirmar comportamiento cuando se procesa una señal sin regla.
- [x] Confirmar comportamiento cuando se intenta exceder capacidad.

## Criterio de cierre

Configuraciones inválidas no pueden introducir silenciosamente un estado ambiguo en runtime.

---

# 11. Definir completamente la semántica de severidad y faults

## FaultSeverity

- [ ] Documentar semántica de `NONE`.
- [ ] Documentar semántica de `WARNING`.
- [ ] Documentar semántica de cualquier severidad superior existente.
- [ ] Definir exactamente cómo cada severidad afecta `FaultSummary`.
- [ ] Definir exactamente cómo cada severidad afecta `Control`.
- [ ] Definir exactamente cómo cada severidad afecta `EcuStateMachine`.
- [ ] Revisar especialmente si `WARNING` tiene comportamiento real o únicamente existe nominalmente.
- [ ] Eliminar severidades sin comportamiento/requisito o implementar la semántica requerida.

## FaultRecord

- [ ] Definir formalmente los invariantes de `FaultRecord`.
- [ ] Confirmar relación uno-a-uno con `SignalId`.
- [ ] Confirmar que no duplique innecesariamente datos de `EvaluationRule`.
- [ ] Confirmar que represente estado runtime y no configuración.

## FaultSummary

- [ ] Definir formalmente cada contador/flag de `FaultSummary`.
- [ ] Confirmar que `activeFaultCount` cuente faults diagnósticos y no señales inválidas por otra definición.
- [ ] Confirmar comportamiento con múltiples severidades simultáneas.
- [ ] Confirmar comportamiento con faults latched y recuperados.

## Criterio de cierre

Cada estado y severidad tiene significado operacional inequívoco y verificable.

---

# 12. Reconstruir y alinear tests

> Esta fase permanece deliberadamente aplazada hasta estabilizar la arquitectura productiva.

- [ ] Eliminar o actualizar tests correspondientes a arquitectura eliminada.
- [ ] Eliminar referencias de tests a tipos eliminados.
- [ ] Crear tests para `Message`.
- [ ] Crear tests para `MessageManager`.
- [ ] Crear tests para `Gateway`.
- [ ] Crear tests para `Control`.
- [ ] Crear tests para `EvaluationRule`.
- [ ] Crear tests para `FaultStateMachine`.
- [ ] Crear tests para `FaultManager`.
- [ ] Crear tests para `EcuStateMachine`.
- [ ] Crear tests para `FaultConfiguration`.
- [ ] Crear tests para validación de `SystemConfig`.
- [ ] Crear tests para IDs duplicados.
- [ ] Crear tests para límites min/max.
- [ ] Crear tests para timeout.
- [ ] Crear tests para regresión/wraparound temporal.
- [ ] Crear tests para confirmation/recovery.
- [ ] Crear tests para latching.
- [ ] Crear tests para severidades.
- [ ] Crear tests para roles de shutdown.
- [ ] Crear tests de pipeline completo del CORE.
- [ ] Crear tests que demuestren que cambiar `SystemConfig` cambia la política sin modificar otro lugar.
- [ ] Crear tests que demuestren que no existe una segunda ruta diagnóstica.
- [ ] Crear tests de capacidad y límites de estructuras fijas.
- [ ] Integrar los tests en CMake.
- [ ] Decidir conscientemente si los tests deben integrarse también en Makefile.
- [x] Confirmar que los tests no formen parte del binario productivo.

## Criterio de cierre

Los invariantes arquitectónicos críticos tienen pruebas automáticas y los tests describen únicamente la arquitectura vigente.

---

# 13. Alinear Makefile y CMake

## Makefile

- [x] Agregar `core/include` al include path.
- [x] Definir `CORE_SRC_DIR`.
- [x] Definir `CORE_INCLUDE_DIR`.
- [x] Mover los objetos del CORE a las nuevas rutas.
- [x] Eliminar `config.o` de `CORE_OBJECTS`.
- [x] Mantener `config.o` como parte del simulador/aplicación.
- [x] Eliminar `signal_store.o` de `CORE_OBJECTS`.
- [x] Confirmar que el simulador no utiliza `SignalStore`.
- [x] Eliminar la regla innecesaria de `signal_store.o` del Makefile del simulador.
- [x] Corregir dependencias de headers trasladados.
- [x] Ejecutar `make clean && make`.
- [x] Confirmar compilación limpia desde cero.

## CMake

- [x] Actualizar fuentes de `ecu_core` a `core/src`.
- [x] Actualizar include público de `ecu_core` a `core/include`.
- [x] Eliminar `config.cpp` de `ecu_core`.
- [x] Incorporar `config.cpp` al ejecutable del simulador.
- [x] Eliminar `signal_store.cpp` de `ecu_core`.
- [x] Eliminar `SignalSample/SignalStore` de los headers públicos del CORE mientras están bajo auditoría.
- [x] Eliminar `config.hpp` de los headers públicos del CORE.
- [x] Mantener compilación aislada de los headers públicos reales del CORE.
- [x] Ejecutar configuración CMake limpia.
- [x] Compilar `ecu_core`.
- [x] Compilar `ecu_simulator`.
- [x] Compilar individualmente todos los headers públicos del CORE.
- [x] Ejecutar el simulador generado por CMake.
- [x] Confirmar comportamiento funcional básico después de la reorganización.

## Consistencia

- [x] Confirmar que Makefile y CMake compilen el mismo conjunto de `.cpp` del CORE.
- [x] Confirmar que ambos utilicen `core/include`.
- [x] Confirmar que `config.cpp` permanezca fuera de la biblioteca CORE en ambos sistemas.
- [x] Confirmar que `SignalStore` no sea incorporado accidentalmente al CORE por ninguno de los dos sistemas.
- [ ] Repetir esta comparación después de resolver definitivamente `SignalSample/SignalStore`.
- [ ] Repetir esta comparación en la auditoría final.

## Criterio de cierre

Makefile y CMake producen el mismo CORE a partir de las mismas fuentes y headers.

---

# 14. Alinear STM32 con el CORE definitivo

> STM32 debe adaptarse al CORE. El CORE no debe deformarse para conservar una implementación provisional de STM32.

- [ ] Esperar a cerrar las decisiones arquitectónicas anteriores antes de modificar el pipeline STM32.
- [ ] Inventariar dependencias actuales de `app/stm32`.
- [ ] Revisar uso actual de `SignalSample`.
- [ ] Revisar uso actual de `SignalStore`.
- [ ] Revisar `signal_acquisition`.
- [ ] Revisar `control_test`.
- [ ] Revisar scripts STM32.
- [ ] Corregir referencias obsoletas de scripts, incluyendo referencias a archivos inexistentes como `src/signal_sample.cpp` si continúan presentes.
- [ ] Definir frontera definitiva `platform/acquisition -> CORE`.
- [ ] Adaptar ADC/TPS a esa frontera.
- [ ] Adaptar NTC/temperatura a esa frontera.
- [ ] Mantener conversiones físicas en la capa apropiada.
- [ ] Evitar introducir ADC, GPIO o HAL dentro del CORE.
- [ ] Garantizar que STM32 utilice `Message`.
- [ ] Garantizar que STM32 utilice `Gateway`.
- [ ] Garantizar que STM32 utilice `Control`.
- [ ] Garantizar que STM32 utilice el mismo `FaultManager`.
- [ ] Garantizar que STM32 utilice el mismo `EcuStateMachine`.
- [ ] Confirmar que el significado de `SignalStatus` sea idéntico en host y STM32.
- [ ] Confirmar que el significado de faults y severidades sea idéntico en host y STM32.
- [ ] Confirmar que solamente cambie el mecanismo de adquisición.
- [ ] Compilar para STM32 sin modificar fuentes del CORE.
- [ ] Validar nuevamente OPERATIONAL.
- [ ] Validar DEGRADED.
- [ ] Validar SAFE_STATE.
- [ ] Validar físicamente el LED rojo de SAFE_STATE que continúa pendiente.
- [ ] Reanudar la prueba de sobretemperatura únicamente después de completar la alineación arquitectónica.

## Criterio de cierre

```text
Consola ----\
             \
              adquisición/adaptación
             /
STM32 ------/
                 |
                 v
          mismo contrato CORE
                 |
                 v
Message -> Gateway -> Control -> FaultManager/EcuStateMachine
```

Las diferencias de hardware no crean una segunda arquitectura funcional.

---

# 15. Actualizar documentación

- [x] Revisar `README.md`.
- [ ] Revisar `docs/ARCHITECTURE.md`.
- [x] Revisar `docs/CURRENT_BEHAVIOR.md`.
- [ ] Revisar `docs/STM32_VALIDATION.md`.
- [ ] Revisar `docs/ANALOG_INPUT_SESSION.md`.
- [ ] Revisar roadmap/documentación adicional vigente.
- [ ] Eliminar diagramas que describan pipelines obsoletos.
- [ ] Corregir referencias a `SignalStore` según su clasificación definitiva.
- [ ] Corregir referencias a arquitectura RANGE/TIMEOUT eliminada.
- [x] Corregir cifras de memoria que correspondan a una arquitectura anterior.
- [x] Documentar la responsabilidad definitiva de cada componente.
- [ ] Documentar la frontera adquisición/CORE.
- [x] Documentar la fuente única de configuración.
- [x] Documentar la estructura `core/include` y `core/src`.
- [ ] Confirmar que la documentación no contradiga el código.

## Criterio de cierre

Un desarrollador que lea la documentación obtiene la misma arquitectura que existe en el código.

---

# 16. Auditoría final de duplicidad y código muerto

## Funcionalidad

- [ ] Buscar funciones con responsabilidad equivalente.
- [ ] Buscar clases que implementen la misma decisión.
- [ ] Buscar validaciones repetidas de rango.
- [ ] Buscar validaciones repetidas de timeout.
- [ ] Buscar decisiones repetidas de severidad.
- [ ] Buscar decisiones repetidas de estado ECU.
- [ ] Buscar rutas diagnósticas paralelas.

## Datos

- [ ] Buscar IDs duplicados.
- [ ] Buscar rangos duplicados.
- [ ] Buscar timeouts duplicados.
- [ ] Buscar severidades duplicadas.
- [ ] Buscar políticas de confirmation/recovery duplicadas.
- [ ] Buscar estados runtime duplicados.
- [ ] Buscar metadata redundante entre estructuras.

## Código muerto

- [ ] Buscar clases sin consumidores.
- [ ] Buscar funciones sin consumidores.
- [ ] Buscar métodos sin consumidores.
- [ ] Buscar enums sin consumidores.
- [ ] Buscar campos sin consumidores.
- [ ] Buscar archivos `.cpp` sin target de compilación justificado.
- [ ] Buscar headers sin consumidores.
- [ ] Buscar scripts obsoletos.
- [ ] Buscar configuraciones CMake obsoletas.
- [ ] Buscar reglas Makefile obsoletas.

## Hardcoding

- [ ] Buscar `SignalId` hardcodeados.
- [ ] Buscar valores físicos hardcodeados.
- [ ] Buscar tiempos hardcodeados.
- [ ] Buscar severidades hardcodeadas.
- [ ] Clasificar cada hardcoding como constante legítima, adaptación de plataforma, dato de simulación o defecto arquitectónico.
- [ ] Eliminar hardcoding que represente configuración/política duplicada.

## Criterio de cierre

No existe duplicidad conocida de funcionalidad, datos, configuración o estado que carezca de una justificación explícita.

---

# 17. Auditoría final de estructura del repositorio

- [ ] Inventariar todos los archivos productivos.
- [ ] Clasificar cada archivo como CORE, aplicación, plataforma, simulación, configuración, test, documentación o herramienta.
- [ ] Confirmar que todos los archivos CORE estén bajo `core/`.
- [ ] Confirmar que ningún archivo no-CORE esté bajo `core/`.
- [ ] Confirmar que no existan `.cpp` productivos huérfanos.
- [ ] Confirmar que no existan headers productivos huérfanos.
- [ ] Confirmar que cada `.cpp` productivo tenga un motivo explícito para pertenecer a un target.
- [ ] Confirmar que Makefile refleje la clasificación.
- [ ] Confirmar que CMake refleje la clasificación.
- [ ] Confirmar que STM32 refleje la clasificación.
- [ ] Confirmar que documentación refleje la clasificación.

## Criterio de cierre

La estructura física del repositorio comunica correctamente su arquitectura.

---

# 18. Verificación final de arquitectura

Antes de declarar terminada la purga:

- [x] `Message` tiene responsabilidad inequívoca.
- [x] `MessageManager` tiene responsabilidad inequívoca.
- [x] `Gateway` tiene responsabilidad inequívoca.
- [x] `Control` tiene responsabilidad inequívoca.
- [x] `EvaluationRule` tiene responsabilidad inequívoca.
- [x] `FaultStateMachine` tiene responsabilidad inequívoca.
- [x] `FaultManager` tiene responsabilidad inequívoca.
- [x] `EcuStateMachine` tiene responsabilidad inequívoca.
- [x] `FaultConfiguration` tiene responsabilidad inequívoca.
- [x] `DiagnosticStatus` tiene responsabilidad inequívoca.
- [ ] `SignalSample` tiene clasificación definitiva.
- [ ] `SignalStore` tiene clasificación definitiva.
- [x] Existe una única identidad canónica de señal en el CORE.
- [x] Existe una única fuente autoritativa de configuración.
- [x] Existe una única evaluación de rango.
- [x] Existe una única evaluación de timeout.
- [x] Existe una única gestión temporal de faults.
- [x] Existe una única decisión de estado global de ECU.
- [ ] Existe una única ruta diagnóstica oficial.
- [ ] No existen configuraciones sin efecto.
- [ ] No existen campos redundantes conocidos.
- [x] No existen tipos obsoletos conocidos.
- [ ] No existen archivos productivos obsoletos conocidos.
- [x] No existen dependencias de plataforma dentro del CORE.
- [x] El CORE compila aisladamente.
- [x] El CORE compila con perfil embedded.
- [x] El CORE compila con el toolchain ARM objetivo.
- [x] Makefile y CMake describen el mismo CORE.
- [x] Consola utiliza el pipeline oficial.
- [ ] STM32 utiliza el pipeline oficial.
- [ ] Tests corresponden a la arquitectura vigente.
- [ ] Documentación corresponde a la arquitectura vigente.
- [x] Build limpio Make pasa.
- [x] Build limpio CMake pasa.
- [ ] Build STM32 pasa.
- [ ] Tests pasan.
- [x] Simulación funcional pasa.
- [ ] Validación STM32 pasa.

---

# Definición de terminado — CORE Architecture Purge

La purga arquitectónica **NO se considera terminada** únicamente porque el proyecto compile.

Solo puede declararse terminada cuando:

1. todos los ítems aplicables de este checklist estén resueltos;
2. cualquier ítem deliberadamente no aplicable tenga una justificación documentada;
3. no exista ninguna duplicidad conocida sin justificar;
4. no exista ninguna segunda fuente de verdad conocida;
5. no exista ninguna ruta diagnóstica paralela conocida;
6. todas las responsabilidades estén definidas;
7. `SignalSample` y `SignalStore` tengan una decisión arquitectónica definitiva;
8. el comportamiento temporal esté definido y verificado;
9. la independencia de plataforma esté demostrada;
10. Makefile, CMake y STM32 estén alineados;
11. los tests representen la arquitectura final;
12. la documentación represente la arquitectura final;
13. una última auditoría global del repositorio no encuentre residuos arquitectónicos pendientes.

Cuando estas condiciones se cumplan, podrá marcarse:

```text
CORE ARCHITECTURE PURGE: COMPLETE
```

Hasta entonces:

```text
CORE ARCHITECTURE PURGE: IN PROGRESS
```
