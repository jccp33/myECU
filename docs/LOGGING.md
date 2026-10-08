# Registro TXT y CSV del simulador

## Alcance

`Logger` está implementado en `include/logger.hpp` y `src/logger.cpp`.
Pertenece al soporte de host y se utiliza desde `randomSimulation()` en
`src/simulations.cpp`. Make y CMake incluyen su fuente en el simulador;
`ecu_core` y el firmware STM32 no dependen del logger. El modo manual no
escribe registros.

## Uso y archivos

Desde el directorio de trabajo deseado, con el simulador compilado:

```bash
./ecu -auto       # logs/txt/<timestamp>.txt: transiciones y causas
./ecu -auto -csv  # logs/csv/<timestamp>.csv: muestras por ciclo
```

`-csv` debe ser el segundo argumento, después de `-auto`. En esa posición,
cualquier otro argumento conserva TXT; no hay validación explícita de opciones.
`main.cpp` pasa `fileType = 1` para CSV y `0` para TXT a `randomSimulation()`.

El simulador crea `logs/` y el subdirectorio elegido respecto al directorio
de trabajo del proceso. Usa `mkdir(path, 0755)`, una API POSIX. Git ignora
la carpeta `logs/`; la regla anterior `*.log` fue sustituida, por lo que
los archivos `.log` fuera de esa carpeta ya no quedan excluidos por esa regla.

El nombre se obtiene de `get_timestamp_ms()`, basado en
`std::chrono::steady_clock`: no es fecha/hora de calendario ni tiempo contado
desde el inicio de la simulación. El origen depende de la implementación del
reloj. Normalmente cada ejecución crea un archivo diferente, pero no hay
unicidad garantizada. La apertura sigue siendo append (`"a"`): si coincide
el nombre, se añaden datos al archivo existente, incluida otra cabecera en CSV.
No hay rotación ni límite de tamaño.

Para observar un archivo concreto desde otra terminal, por ejemplo:

```bash
tail -f logs/txt/18259727.txt
```

## Formato TXT

Ejemplo de `logs/txt/18259727.txt`:

```text
[18259731 ms] INIT -> SELF_TEST
[18260235 ms] SELF_TEST -> OPERATIONAL
```

Se escribe cuando cambia `EcuState`, después de procesar el ciclo, incluida
la transición final a `SHUTDOWN`. La línea contiene timestamp monotónico,
estado anterior y estado nuevo; no hay una línea independiente para el
estado inicial `INIT`.

Al entrar en `DEGRADED` o `SAFE_STATE`, `logFaultCauses()` recorre las señales
y agrega las que tienen un registro `CONFIRMED`, `RECOVERING` o `LATCHED`.
Cada línea muestra nombre, estado inmediato de señal, estado temporal del
fallo y último valor con dos decimales y unidad. Se incluyen todos esos fallos
activos, no solo los responsables de la transición. Un registro `RECOVERING`
puede tener una señal ya válida; un `LATCHED` puede persistir después de
desaparecer la condición física. En timeout se muestra la última muestra.

No se registran cambios individuales de fallo si el estado global permanece
igual. Un `SAFE_STATE` por diagnóstico interno puede carecer de líneas de
causas, pues estas proceden exclusivamente de `FaultManager`.

## Formato CSV

La primera línea contiene `<nombre>(<unidad>)` para cada señal configurada,
en orden de configuración, y la columna final `ECU_STATE`. Actualmente son
once señales y el estado global. El separador emitido es coma seguida de espacio.

Cada ciclo escribe los valores de `Message::getRawValue()` con formato fijo
y cuatro decimales, seguidos del estado resultante de `Control`. Se incluye
el ciclo que alcanza `SHUTDOWN`; no se exige una transición para emitir fila.
La pausa nominal entre ciclos es 500 ms, más procesamiento y E/S.

CSV no incluye timestamp, edad de muestra, validez ni causa individual de
fallo. Una señal con timeout puede mostrar su última muestra almacenada;
un valor numérico no demuestra que la señal sea válida. Los nombres y unidades
se escriben sin escape ni comillas CSV; si en el futuro contienen comas,
comillas o saltos de línea, el formato necesitará adaptación.

## Ciclo de vida e interfaz

- `Logger()` inicializa el archivo como cerrado.
- `open(filename)` devuelve si pudo abrirlo; devuelve `false` si ya está abierto.
- `write(message)` agrega una línea y llama a `fflush()` después de escribir.
- `close()` cierra el archivo; el destructor también llama a `close()`.

`write()` no hace nada si el archivo está cerrado o el mensaje es nulo.
`randomSimulation()` no comprueba los resultados de creación de carpetas ni
`open()`: si la apertura falla, la simulación continúa sin registro. El helper
acepta `EEXIST` sin verificar que la ruta existente sea un directorio.
Los resultados de `fprintf()`, `fflush()` y `fclose()` no se propagan.

`fflush()` facilita observar las líneas durante la ejecución, pero no garantiza
persistencia ante un corte de energía. El destructor cierra el archivo durante
la salida normal, como el retorno al alcanzar `SHUTDOWN`; no debe asumirse
ese cierre ante terminación abrupta. CSV escribe y vacía en cada ciclo, por
lo que genera más E/S y crecimiento que el registro condicionado a transiciones.

## Límites y validación

- El nombre por ejecución no sustituye un identificador único de sesión.
- No hay DTC persistentes, freeze-frames ni diagnóstico por señal en CSV.
- La clase posee un `FILE*` y no bloquea la copia. Debe usarse como objeto local
  sin copiar ni asignar; la simulación actual cumple esa condición.
- No hay pruebas automatizadas específicas de los formatos, creación de carpetas,
  colisiones de nombres, errores de E/S ni integración con el bucle interactivo.
- La revisión documental del 7 de octubre de 2026 observó un TXT de 100 líneas
  y un CSV de 502 líneas, incluida su cabecera. Son evidencia de ejecuciones
  anteriores, no una nueva validación del ejecutable o del código actual.

Posibles mejoras: reportar errores de E/S, impedir copias del logger, garantizar
nombres únicos, agregar tiempo y diagnóstico al CSV y registrar cambios de
fallo sin transición global. Véase [TESTING.md](TESTING.md).
