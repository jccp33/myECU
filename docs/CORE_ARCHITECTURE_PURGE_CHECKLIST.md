# myECU — CORE Architecture Purge Checklist

## Objetivo

Garantizar que el CORE de **myECU** cumpla con los siguientes principios arquitectónicos:

- No exista duplicidad injustificada de funcionalidades.
- No exista duplicidad injustificada de datos o fuentes de configuración.
- Cada componente tenga una responsabilidad claramente definida.
- Las dependencias entre componentes sean explícitas, justificadas y unidireccionales.
- El CORE sea completamente independiente de Linux, Windows, STM32, AVR u otra plataforma.
- El CORE pueda compilarse sin modificar su código fuente en los toolchains objetivo.
- La simulación por consola y STM32 utilicen exactamente el mismo pipeline del CORE.
- Makefile y CMake describan el mismo CORE.
- Los tests demuestren las propiedades arquitectónicas anteriores.

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
- [x] Identificar referencias obsoletas en README y documentación para actualizarlas posteriormente.

## Criterio de cierre

Ningún tipo, clase, archivo o dependencia correspondiente a una arquitectura eliminada permanece dentro del CORE productivo.

---

# 2. Garantizar una única fuente de verdad para cada dato

## Identidades

- [x] Auditar `messageId` frente a `SignalId.id`.
- [x] Eliminar `messageId` si representa exactamente la misma identidad que `SignalId.id`.
- [x] Auditar `SensorId` frente a `SignalId`.
- [x] Documentar claramente la responsabilidad de `SensorId` si debe conservarse.
- [x] Documentar claramente la responsabilidad de `SignalId`.

## Configuración diagnóstica

- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `minValue`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `maxValue`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `timeoutMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `confirmationTimeMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `recoveryTimeMs`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `FaultSeverity`.
- [x] Confirmar que `SystemConfig` sea la fuente autoritativa de `FaultLatching`.

## Datos derivados

- [x] Auditar `Message::severity`.
- [x] Eliminar `Message::severity` si ningún consumidor necesita esa copia.
- [x] Confirmar que `EvaluationRule` sea exclusivamente una representación derivada de `SystemConfig`.
- [x] Impedir que `EvaluationRule` introduzca una segunda configuración independiente.
- [x] Fortalecer `validateEvaluationRuleSet()` o restringir la API para impedir configuraciones divergentes.

## Configuración potencialmente muerta

- [x] Definir formalmente la semántica de `SystemConfig::maxInvalidSignals`.
- [x] Determinar que `maxInvalidSignals` no forma parte del diseño funcional actual.
- [x] Eliminar `maxInvalidSignals` al no existir un requisito funcional que lo justifique.

## Criterio de cierre

Modificar cualquier política de una señal requiere modificar un único lugar.

Ejemplo:

```text
SystemConfig
    |
    +--> Message / Gateway
    |
    +--> EvaluationRule
             |
             +--> FaultManager