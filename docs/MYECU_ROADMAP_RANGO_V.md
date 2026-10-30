# Alineación de myECU con Hito Rango V

Fecha de elaboración: 30 de octubre de 2026.

Responsable: **JESUS CARLOS CARDENAS**.

Referencia: `Hito_Rango_V_ECU_Gateway_Control.pdf`, 28 páginas,
«Optimización, serialización, persistencia y testing».
El PDF es material externo y no está incluido en el repositorio.

## Objetivo y alcance

Completar los requisitos del Rango V sobre el proyecto existente, conservando
la separación entre Gateway, Control, simulación, persistencia y plataforma.
Este documento es un plan: las casillas pendientes no representan funciones
implementadas. Los nombres de componentes nuevos son propuestas.

La revisión de partida encontró compilación host con Make/CMake, doce
ejecutables de prueba aprobados, simulación automática con TXT/CSV y
compilación ARM de las aplicaciones de control y RPM. Persisten limitaciones
de integración RPM y del script de LEDs. La compilación no sustituye pruebas
físicas.

El Rango V no requiere CAN, UDS, SOME/IP, RTOS ni cerrar la adquisición física
RPM. Esas tareas permanecen en los otros roadmaps y no deben bloquear esta
entrega de host. Completar Rango V tampoco las da por resueltas.

## Reglas de implementación

- Conservar el estilo del código existente y realizar cambios pequeños y verificables.
- Gateway calcula validez; Control interpreta criticidad y decide estados.
- Logger y lectores no cambian la FSM ni recalculan la validez de señales.
- Mantener filesystem, benchmarks y presentación fuera de `core/`.
- No duplicar reglas de rango y tiempo para cada formato o escenario.
- No agregar contenedores ni algoritmos únicamente para demostrar su uso.
- Conservar las once señales; las cinco del PDF son el mínimo de aceptación.
- Diferenciar el perfil del hito del perfil experimental STM32 cuando sus
  políticas sean distintas. No cambiar silenciosamente el comportamiento físico.
- Preservar los diagnósticos temporales existentes: una señal inválida y un
  fallo confirmado son conceptos diferentes.

## Matriz de trazabilidad

| Requisito del PDF | Páginas | Trabajo previsto | Evidencia de cierre |
|---|---|---|---|
| Responsabilidades y criticidad | 3–6, 12 | Fases 1 y 3 | Configuración y escenarios de aceptación |
| Vigencia por ciclos | 7–8 | Fase 3 | Frontera Nmax y recuperación sin pausas |
| Complejidad y contenedores | 9–10 | Fase 4 | Análisis de tres operaciones y decisiones |
| Lambdas y algoritmos STL | 11 | Fase 4 | Operaciones reales refactorizadas y regresión |
| Factor/offset y representación | 13–14 | Fase 5 | Vectores de codificación, límites y redondeo |
| Binario, versión e integridad | 15–17 | Fase 6 | Especificación, bytes conocidos y rechazo de corrupción |
| CSV y binario persistentes | 18 | Fases 6–7 | Escritura/lectura del mismo registro lógico |
| Comparación experimental | 19 | Fase 9 | Mediciones reproducibles y tabla CSV/binario |
| Testing, fixtures y fake/mock | 20–23 | Fases 2, 3 y 8 | CTest y matriz de escenarios |
| C++17, CMake y Debug | 24 | Fase 2 | Construcción limpia y ejecución CTest |
| Entregables y reproducción | 25–28 | Fase 10 | README, comandos y evidencia de ejecución |

## Fase 1 — Fijar el contrato de aceptación

Dependencias: ninguna. Esta es la primera tarea activa.

- [ ] Registrar el commit de partida y conservar el resultado de las pruebas existentes.
- [ ] Definir un perfil explícito `Rango V`, seleccionable en ejecución o
  construcción, y documentar cómo se elige. El mecanismo exacto se decide
  antes de implementar para no dispersar condicionales en el núcleo.
- [ ] Mantener el perfil experimental existente para STM32 y sus pruebas.
- [ ] Definir en el perfil Rango V la política siguiente:

| Señal | Severidad de Control | Política de recuperación propuesta |
|---|---|---|
| Velocidad | `DEGRADED` | Recuperable |
| RPM | `DEGRADED` | Recuperable |
| Temperatura | `CRITICAL` | Recuperable |
| Batería | `DEGRADED` | Recuperable |
| Presión de aceite | `CRITICAL` | Recuperable |

- [ ] Documentar que RPM y batería difieren del perfil actual, donde son
  críticas y batería es latched. No mantener latching de batería en el
  escenario normal del hito.
- [ ] Documentar la correspondencia `VALID → VALID`,
  `OUT_OF_RANGE → INVALID`, `TIMEOUT → MISSING` y el tratamiento adicional
  de `UNDEFINED` como error diagnóstico, sin ocultarlo como señal válida.
- [ ] Mantener `SHUTDOWN_REQ` como extensión justificada de los estados pedidos.
- [ ] Definir duración de confirmación y recuperación, paso del reloj de
  diagnóstico y momento exacto en que se espera cada transición.
- [ ] Definir el estado de una señal que todavía nunca ha recibido una muestra:
  no presentar el valor inicial de configuración como recepción real.

Archivos afectados: `src/config.cpp`, `include/config.hpp`, tipos de dominio
si son necesarios, pruebas de configuración y documentación.

**Cierre:** el contrato explica sin ambigüedad por qué batería inválida produce
`DEGRADED` en el hito y cómo se conserva el comportamiento experimental.

## Fase 2 — C++17 y pruebas integradas con CTest

Dependencia: fase 1 para conocer los perfiles que deben verificarse.

- [ ] Declarar C++17 explícitamente en los targets host, no depender de la
  versión predeterminada del compilador.
- [ ] Alinear Make y el script de pruebas con la configuración host C++17.
- [ ] Decidir y documentar si los scripts ARM permanecen en C++11 como perfil
  compatible adicional o migran a C++17. Verificar el toolchain antes de migrarlos.
- [ ] Incorporar `include(CTest)` y respetar `BUILD_TESTING`.
- [ ] Registrar con `add_test` los doce ejecutables actuales, reutilizando
  `ecu_core` y agrupando las fuentes auxiliares que realmente necesiten.
- [ ] Mantener operativo `scripts/run_tests.sh` durante la transición.
- [ ] Conservar las comprobaciones aisladas de headers y el perfil freestanding.
- [ ] Verificar compilación limpia Debug y documentar estas órdenes objetivo:

```bash
cmake -S . -B build-rango-v -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-rango-v
ctest --test-dir build-rango-v --output-on-failure
```

- [ ] Considerar que Make/CMake y distintos perfiles comparten actualmente
  `ecu` y `build/libecu_core.a`: evitar builds simultáneos que se sobrescriban
  y verificar reconstrucción limpia, o introducir aislamiento documentado.

**Cierre:** CTest descubre y ejecuta las pruebas; el build host exige C++17;
no hay regresiones en los perfiles que se conservan.

## Fase 3 — Vigencia por ciclos y decisiones de Control

Dependencias: fases 1–2.

- [ ] Introducir un tipo explícito de ciclo y un `Nmax` configurable.
- [ ] Conservar el ciclo de última recepción y la existencia de una primera
  muestra por señal. Actualizarlos únicamente cuando se recibe un dato.
- [ ] Implementar la política temporal en Gateway o en un componente portable
  usado por Gateway. Mantener la política en milisegundos del perfil STM32.
- [ ] Compartir la evaluación de rango entre políticas; no crear dos Gateway
  independientes con reglas duplicadas.
- [ ] No pasar ciclos disfrazados como `TimestampMs`: separar las unidades del
  contador de recepción y del tiempo usado por confirmación/recuperación.
- [ ] En escenarios de prueba, avanzar explícitamente ambos valores, por
  ejemplo un ciclo y un paso fijo de milisegundos, sin `sleep`.
- [ ] Implementar `edad > Nmax → MISSING`; en `edad == Nmax` aún está vigente.
- [ ] Definir tratamiento de regresión y desbordamiento del contador.
- [ ] Conservar el último valor al faltar muestras; reevaluar una muestra nueva.
- [ ] Rechazar valores no finitos y configuraciones de rango inválidas.
- [ ] Exigir señal válida para interpretar solicitud y permiso de apagado.
- [ ] Revisar que errores internos, incluido conteo de mensajes inválido,
  no dejen al sistema aparentando un estado saludable.

Pruebas obligatorias: última recepción en ciclo 10, `Nmax = 3`, vigente en
10–13, ausente en 14; recepción válida en 15; recepción fuera de rango en 15;
ausencia desde el arranque; límite cero; regresión/desbordamiento según contrato.

Archivos afectados: `core/src/getaway.cpp`, `core/include/message.hpp`,
`core/src/mssgmanager.cpp`, configuración temporal, `core/src/control.cpp` y tests.

**Cierre:** Gateway detecta ausencia por ciclos y Control consume el resultado;
las pruebas no dependen de tiempo real y el timeout STM32 conserva su semántica.

## Fase 4 — STL y eficiencia con propósito

Dependencia: pruebas de regresión de fases 2–3.

- [ ] Analizar expresamente búsqueda por identidad, validación de todas las
  señales y conteo de `INVALID`/`MISSING`: tiempo, memoria y recorrido real.
- [ ] No confundir conteo de señales inválidas con conteo de faults confirmados.
- [ ] Usar `find_if` con predicado para una búsqueda real por `SignalId`.
- [ ] Usar `count_if` para el resumen de invalidez/ausencia del dashboard o registro.
- [ ] Aplicar `any_of`/`all_of` a una condición real donde mejore claridad.
- [ ] Evaluar `transform` para convertir registros; usarlo solo si aporta claridad.
- [ ] Respetar el rango lógico `[begin, begin + sensorCount)`, no recorrer como
  señales activas toda la capacidad de `std::array`.
- [ ] Comparar `array`, `vector`, `unordered_map`, `map`, `set` y `queue` en
  relación con las operaciones existentes. Justificar usos y descartes.
- [ ] Explicar que sustituir un bucle por `find_if` no cambia O(n).
- [ ] Evaluar eliminar búsquedas repetidas mensaje → regla mediante índices
  estables preparados al configurar; implementar solo si el beneficio y
  la preservación de identidades/capacidad quedan demostrados.
- [ ] Actualizar el análisis del README con cambios reales, sin atribuir
  mejoras asintóticas a cambios puramente de estilo.

**Cierre:** tres operaciones analizadas, lambdas/predicados utilizados en
operaciones reales, decisiones de contenedores documentadas y pruebas verdes.

## Fase 5 — Escalamiento reversible

Dependencias: contrato de configuración y pruebas disponibles.

- [ ] Crear un componente portable de codec separado de ADC y de Logger.
- [ ] Implementar `physical = raw * factor + offset` y la inversa con factor
  distinto de cero, sin convertir a entero antes de comprobar límites.
- [ ] Definir al menos dos señales con factores y offsets distintos.
  Propuesta: temperatura `uint16_t`, F=0.1, O=-40; RPM `uint16_t`, F=0.25, O=0.
- [ ] Especificar ancho, signo, unidades y rango representable de cada codec.
- [ ] Diferenciar rango de representación de rango válido para Gateway: una
  señal inválida para el vehículo puede ser perfectamente serializable.
- [ ] Definir redondeo explícito, por ejemplo al más cercano con empates hacia
  arriba para raw no negativo, y saturación antes del cast.
- [ ] Devolver un resultado explícito para éxito, saturación y entrada inválida.
  No transformar silenciosamente NaN/infinito en una muestra normal.
- [ ] Probar el vector temperatura raw=1255 → 85.5 °C y su operación inversa.
- [ ] Probar extremos, valores intermedios, empates, fuera de representación,
  factor cero y valores no finitos. Usar tolerancia ligada al paso de cuantización.
- [ ] Definir representación para las demás señales que incluirá la persistencia.

**Cierre:** ambos codecs tienen vectores conocidos y round-trip documentado;
la saturación no se confunde con una lectura fiel al valor original.

## Fase 6 — Registro y serialización binaria segura

Dependencia: fase 5; contrato de estados de fase 1.

- [ ] Diseñar un registro lógico independiente de TXT, CSV y archivo binario.
- [ ] Incluir ciclo, perfil/configuración relevante, valores de señales,
  resultados de Gateway y estado de Control. Añadir unidades/escala mediante
  una especificación versionada o metadatos inequívocos.
- [ ] Definir qué se conserva para muestras ausentes y para valores no
  representables; guardar el resultado de codificación si hubo saturación.
- [ ] Publicar una tabla de campos con offsets, tamaños, tipos y significado.
- [ ] Elegir un endian explícito, por ejemplo little-endian, y códigos de estado
  estables; no depender de padding, ABI ni valores futuros de enums internos.
- [ ] Definir identificación/versionado y delimitación de registros: longitud
  fija documentada o longitud validada con un máximo antes de leer/asignar.
- [ ] Codificar campo a campo con enteros de tamaño fijo y operaciones de bytes.
  No volcar la memoria de una struct ni serializar punteros.
- [ ] Implementar checksum de suma de 8 bits o un CRC especificado por completo.
  Para suma, definir bytes cubiertos, módulo 256 y posición del checksum.
- [ ] Verificar integridad antes de publicar un registro decodificado.
- [ ] Rechazar versión desconocida, códigos inválidos, longitudes incoherentes,
  registros truncados y alteraciones; distinguir EOF limpio de truncamiento.
- [ ] Definir política tras corrupción: detener e informar es aceptable si se
  documenta; no prometer resincronización sin implementarla y probarla.

Pruebas: bytes esperados de un registro conocido, round-trip, todos los puntos
de truncamiento de ese registro, cambio de un byte, checksum incorrecto,
versión/códigos inválidos y varios registros consecutivos.

**Cierre:** el formato está documentado y es independiente de la plataforma;
los datos malformados no producen registros aparentemente válidos.

## Fase 7 — Persistencia CSV y binaria

Dependencias: fase 6 y resultados Gateway/Control de fase 3.

- [ ] Generar un único snapshot después de procesar cada ciclo y entregarlo a
  ambos escritores. No ejecutar dos simulaciones aleatorias para compararlos.
- [ ] Ampliar CSV con ciclo, resultados de Gateway por señal y estado de Control.
- [ ] Definir cabecera, precisión, locale y escape de campos CSV.
- [ ] Implementar lectores CSV y binario con resultados de error explícitos.
- [ ] Conservar TXT de eventos como función adicional, no como sustituto del binario.
- [ ] Comprobar creación de directorios, apertura, escritura, flush y cierre.
- [ ] Impedir copias del Logger propietario de `FILE*`, o definir propiedad
  segura; mantener cierre automático en todas las salidas normales.
- [ ] Garantizar nombres de sesión sin colisiones y política de append/overwrite.
- [ ] Evitar que errores de persistencia cambien silenciosamente la FSM: informar
  al coordinador y aplicar la política de ejecución documentada.
- [ ] Permitir escribir ambos formatos desde el mismo conjunto de registros.
- [ ] Documentar SQLite conceptualmente: consultas, transacciones, dependencias,
  almacenamiento y coste. Implementarlo es opcional.

**Cierre:** ambos formatos se escriben y leen; contienen todos los datos exigidos
y los errores de E/S son observables. No se introduce filesystem en `core/`.

## Fase 8 — Escenarios reproducibles, fixtures e integración

Dependencias: fases 3 y 7. Añadir pruebas unitarias durante cada fase previa.

- [ ] Crear una fixture común con perfil Rango V, mensajes nominales y paso
  explícito de ciclo/tiempo; restablecer completamente su estado entre casos.
- [ ] Parametrizar fronteras mediante tablas y ejecutar todos los casos aunque
  uno falle, ofreciendo señal, entrada y resultado esperado en el diagnóstico.
- [ ] Usar al menos un fake real: fuente de muestras programada o repositorio
  en memoria que capture registros y permita simular fallo de escritura.
- [ ] Verificar el recorrido Gateway → Control → snapshot → escritor/lector,
  incluidos los estados recibidos por Logger.
- [ ] Añadir escenarios seleccionables, número finito de ciclos y semilla
  configurable. Un seed fijo no sustituye un escenario de aceptación explícito.
- [ ] No usar `sleep`, teclado ni consola interactiva en pruebas o benchmarks.
- [ ] Dar una salida de proceso distinta de cero si un escenario no cumple.

| Escenario | Gateway esperado | Control esperado |
|---|---|---|
| Todas las señales válidas | `VALID` | `INIT → SELF_TEST → OPERATIONAL` según contrato |
| Batería fuera de rango sostenida | `INVALID` | `DEGRADED` tras confirmación |
| RPM fuera de rango sostenida | `INVALID` | `DEGRADED` tras confirmación |
| Aceite fuera de rango sostenido | `INVALID` | `SAFE_STATE` tras confirmación |
| Temperatura sin nuevas muestras | `MISSING` al superar Nmax | `SAFE_STATE` tras confirmación |
| Temperatura recibida de nuevo | Reevaluar valor y vigencia | Recuperación según temporización y resto de fallos |
| Solicitud y permiso válidos | Datos vigentes | `SHUTDOWN_REQ → SHUTDOWN` |
| Permiso vencido | `MISSING` | No autoriza apagado por el valor antiguo |

- [ ] Probar las transiciones de self-test y shutdown según reglas del proyecto;
  distinguir pruebas de la FSM pura de un self-test físico real, aún pendiente.
- [ ] Cubrir escritura/lectura, truncamiento, corrupción, datos no finitos y
  errores del fake de persistencia.
- [ ] Ejecutar CTest y sanitizers en host; conservar los resultados.

**Cierre:** escenarios automatizados reproducibles, fake/mock verificable,
fronteras parametrizadas y regresión aprobada sin depender de pausas reales.

## Fase 9 — Comparación experimental CSV frente a binario

Dependencias: fases 7–8; ambos lectores/escritores funcionan correctamente.

- [ ] Generar una vez un conjunto fijo, por ejemplo 10 000 snapshots, con
  escenarios normales y fallos. Identificarlo por semilla/escenario y digest.
- [ ] Usar exactamente esos snapshots, mismo orden y misma cantidad para ambos
  formatos. No reducir el muestreo CSV en esta comparación controlada.
- [ ] Definir si se comparan valores cuantizados comunes o valores originales;
  comprobar igualdad semántica con tolerancias explícitas cuando corresponda.
- [ ] Medir bytes finales, tiempo de escritura y tiempo de lectura completos.
- [ ] Separar generación de datos de medición de E/S. Documentar si se incluyen
  serialización, apertura, flush y cierre, y usar condiciones comparables.
- [ ] Indicar que `fflush` no garantiza persistencia física; distinguir caché
  caliente/fría si se mide, sin atribuir a disco lo que fue atendido por caché.
- [ ] Repetir mediciones, alternar orden de formatos y registrar dispersión,
  por ejemplo mediana y mínimo/máximo; no basarse en una única ejecución.
- [ ] Registrar CPU, SO, compilador, flags, tipo de build, almacenamiento,
  número de muestras, repeticiones y versiones del formato/configuración.
- [ ] Probar recuperación o rechazo ante truncamiento/corrupción en ambos formatos.
  No afirmar integridad equivalente si CSV no incluye checksum.
- [ ] Entregar tabla comparativa y procedimiento de reproducción.

| Métrica | CSV | Binario |
|---|---|---|
| Número de snapshots | Por medir; idéntico | Por medir; idéntico |
| Tamaño final en bytes | Por medir | Por medir |
| Escritura: mediana y dispersión | Por medir | Por medir |
| Lectura: mediana y dispersión | Por medir | Por medir |
| Inspección humana | Describir | Describir |
| Truncamiento/corrupción | Resultado observado | Resultado observado |
| Fidelidad y cuantización | Especificar | Especificar |

**Cierre:** resultados medidos, metodología reproducible y conclusiones basadas
en evidencia; no asumir de antemano que binario siempre gana en toda métrica.

## Fase 10 — Documentación y entrega

Dependencias: fases anteriores completadas.

- [ ] Actualizar README con el mapeo Rango V, perfiles, criticidad, ciclos,
  codecs, formatos, decisiones STL y limitaciones restantes.
- [ ] Documentar compilación C++17 Debug, CTest, escenarios y benchmarks con
  comandos reales verificados; no presentar nombres propuestos como disponibles.
- [ ] Documentar cómo reproducir todos los escenarios de aceptación sin teclado.
- [ ] Incorporar especificación binaria, tabla experimental y evidencia de fallos.
- [ ] Comprobar construcción y pruebas desde un checkout/directorio limpio,
  sin depender de artefactos locales ni de archivos no versionados.
- [ ] Revisar que las instrucciones no ejecuten flasheo STM32 como parte de
  la validación host del hito.
- [ ] Comprobar enlaces y actualizar los otros roadmaps para reflejar trabajo
  realizado sin declarar cerrados RPM, CAN o seguridad funcional.
- [ ] Preparar un commit/release de entrega con alcance y resultados explícitos.

## Orden de trabajo y criterio final

Orden recomendado: **1 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 9 → 10**.
Las pruebas unitarias se agregan junto a cada cambio; la fase 8 consolida
escenarios e integración, no pospone todo el testing hasta el final.

La siguiente acción concreta es cerrar el contrato de la fase 1 y establecer
C++17/CTest antes de refactorizar o añadir persistencia.

El hito se considera listo cuando todas las categorías de evaluación tienen
evidencia: separación Gateway/Control (20 %), tiempo y estados (20 %), STL y
eficiencia (15 %), escalamiento/serialización (15 %), persistencia/comparación
(15 %) y testing (15 %). Estos porcentajes son la rúbrica del PDF, no una
calificación estimada del proyecto.

Documentos relacionados:

- [Roadmap general](ROADMAP.md)
- [Roadmap de Rango VII](MYECU_ROADMAP_RANGO_VII.md)
- [Logger actual](LOGGING.md)
- [Pruebas actuales](TESTING.md)
- [Arquitectura](ARCHITECTURE.md)
