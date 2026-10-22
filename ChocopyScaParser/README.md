# Scanner y parser ChocoPy C++ — Análisis LR(1)

Herramienta en C++17 que genera la colección canónica de ítems LR(1) y las
tablas **Action** y **Goto** desde un archivo de gramática, y que integra el
scanner de ChocoPy con su gramática para analizar archivos fuente. No usa
Bison ni bibliotecas que construyan autómatas o tablas.

## Funciones

| Enunciado | Método | Qué hace |
|---|---|---|
| 1. `first()` | `computeFirstSets()` | FIRST de todos los símbolos por punto fijo, con `epsilon` para los anulables |
| 2. `firstCadena(β)` | `computeFirstOfString(beta)` | FIRST de una cadena `s1 … sk` |
| 3. `closure(s)` | `computeClosure(s)` | Cerradura de un conjunto de ítems LR(1) |
| 4. `goto(s, x)` | `computeGoto(s, x)` | Conjunto alcanzado desde `s` al reconocer `x` |
| 5. `coleccionCanonica()` | `buildCanonicalCollection()` | `CC = {cc0, cc1, …}` y el registro de transiciones |
| 6. `llenarTablas()` | `fillTables()` | `Action` y `Goto` con detección de conflictos |
| 7. Recuperación en modo pánico | `parseWithPanicRecovery(tokens, N)` | Analiza tokens y resincroniza usando `FOLLOW(A)` para un no terminal `A` de `N` |

`goto` es palabra reservada de C++, por eso el método se llama `computeGoto`.

Un ítem `[A → β • γ, a]` se representa con **tres enteros** (producción,
posición del punto, índice del terminal de anticipación) y un conjunto de
ítems es un `std::set<Item>`, de modo que dos `cc` se comparan con `==`.
Cada celda de Action guarda sus acciones (`s3`, `r2`, `acc`) junto con los
ítems que las generan; si hay más de una acción distinta, es un conflicto.

## Escaneo y análisis de archivos ChocoPy

El ejecutable conecta el scanner con el parser: `--source` lee un archivo,
lo tokeniza y pasa las categorías de tokens directamente a la gramática
`src/chocopy.txt`. El parser calcula sus tablas LR(1) con esa gramática y
analiza el flujo obtenido.

```sh
make
./build/chocopy-parser --source test/casesChocopy/01_hola.py
./build/chocopy-parser --source test/casesChocopy/12_clase.py --panic stmt,expr,block
```

La gramática usa las categorías del scanner (`ID`, `INTEGER`, `PLUS`,
`NEWLINE`, etc.) como terminales. El ejecutable distingue los identificadores
de declaraciones tipadas como `TYPED_ID` en posiciones de declaración para
que estas no se confundan con expresiones durante el análisis LR(1). Si se usa
`--parse` para proporcionar tokens manualmente, se deben usar los nombres de
terminal de la gramática y esa categoría `TYPED_ID` cuando corresponda. Los
errores léxicos se reportan y el análisis sintáctico continúa con los tokens
recuperables; el proceso termina con código distinto de cero si hubo errores
léxicos o sintácticos.

## Análisis con recuperación en modo pánico

`parseWithPanicRecovery(tokens, N)` ejecuta las tablas LR generadas y aplica
el algoritmo de recuperación del documento. `N` es el conjunto de no
terminales de anclaje elegidos por la aplicación; sus conjuntos `FOLLOW` se
calculan automáticamente a partir de la gramática. Ante un error, el parser
desapila estados hasta encontrar un `Goto` para algún símbolo de `N`, descarta
tokens hasta `FOLLOW(A)`, inserta `A` en la pila y continúa. Si no encuentra
un anclaje o llega al fin de entrada sin sincronizarse, aborta. Si se repite
un error en la misma posición, descarta un token para evitar ciclos.

El resultado indica si la entrada fue aceptada o si la recuperación abortó,
cuántos errores sintácticos y tokens descartados hubo, e incluye diagnósticos.
La gramática debe estar cargada y sus conjuntos FIRST, colección canónica y
tablas LR deben haberse construido antes de analizar; las tablas con
conflictos no se pueden ejecutar. La entrada se pasa como nombres de tokens
(por ejemplo, `{"id", "=", "num"}`); el parser agrega `$` automáticamente.

```cpp
parser.computeFirstSets();
parser.buildCanonicalCollection();
if (parser.fillTables() == 0) {
    ParseResult result = parser.parseWithPanicRecovery(
        {"id", "=", "id", "+", "+", "id", ";", "id", "=", "num"},
        {"S", "E", "T"});
}
```

La elección de `N` afecta cuánto código se descarta y qué tan local es la
recuperación. Conviene incluir no terminales que representen unidades
recuperables del lenguaje, por ejemplo sentencias y expresiones.

Para mostrar en la terminal la traza de estados, símbolos, entrada restante y
acciones, ejecuta el programa con `--parse` y define los anclajes con `--panic`
(separados por comas):

```sh
cd ChocoPyScaParser
make
./build/chocopy-parser test/cases/panic_recovery.txt \
  --parse "id = id + + id ; id = num" \
  --panic S,E,T
```

La traza incluye las acciones normales, las celdas `Action` vacías y las
operaciones de recuperación. `--parse` recibe nombres de tokens separados por
espacios, no el código fuente ni los lexemas de los identificadores. `$` se
agrega automáticamente. Sin `--parse`, el ejecutable conserva su modo habitual
de generar e imprimir las tablas.

## Formato de la gramática

Una producción por línea, con `->` (o `::=`) y alternativas con `|`:

```
Goal -> List
List -> List Pair | Pair
Pair -> ( Pair ) | ( )
```

- Los símbolos van separados por espacios.
- **No terminal**: aparece en el lado izquierdo de alguna producción.
  **Terminal**: aparece en el lado derecho, pero no en ningún lado izquierdo.
- La producción vacía se escribe `epsilon`: `B -> b | epsilon`.
- El **símbolo inicial** es el lado izquierdo de la primera regla.
- `eof` es reservado y lo agrega la herramienta (internamente `$`); no se
  escribe en la gramática.
- **Aumentación**: si el símbolo inicial tiene una sola producción y no
  aparece en ningún lado derecho (como `Goal -> List`), esa regla ya es la
  aumentada y se usa tal cual. Si no, la herramienta agrega `S' -> S`.

## Compilar y ejecutar con `make` (Linux, macOS, MinGW)

Desde `ChocoPyScaParser/`:

```sh
make          # compila build/chocopy-parser y build/run_tests
make test     # ejecuta la batería de pruebas
make run      # tablas de src/compiladores.txt
make clean
```

Para otra gramática:

```sh
./build/chocopy-parser test/cases/parentesis.txt
```

## Salida

La herramienta imprime, en este orden:

1. las producciones numeradas;
2. los conjuntos FIRST de todos los símbolos;
3. cada `cc_i` con sus ítems y sus transiciones;
4. las tablas `Action` y `Goto`, separadas por tabuladores (una celda con
   conflicto muestra sus acciones unidas por `/`, p. ej. `r1/s3`);
5. si los hay, los conflictos: tipo (`shift/reduce` o `reduce/reduce`),
   estado, terminal y los ítems que los causan.

Los `cc_i` se procesan en orden de creación y, dentro de cada uno, las
transiciones siguen el orden de primera aparición de los símbolos en la
gramática, sin contar el símbolo inicial. Así la salida es comparable con la
de otros equipos.

**Códigos de salida:** `0` sin conflictos, `1` si hay conflictos, `2` si no se
pudo leer la gramática.

## Caso de referencia

Para la gramática de paréntesis la herramienta produce los 12 conjuntos y
exactamente la tabla del enunciado:

```
estado  eof   (     )     List  Pair
0             s3          1     2
1       acc   s3                4
2       r2    r2
3             s6    s7          5
4       r1    r1
5                   s8
6             s6    s10         9
7       r4    r4
8       r3    r3
9                   s11
10                  r4
11                  r3
```

## Batería de pruebas

`test/cases/` contiene cada gramática (`.txt`) con su tabla esperada
(`.tabla`, separada por tabuladores). Las tablas esperadas se generaron con
una implementación **independiente** del generador y se contrastaron con la
tabla de referencia del enunciado.

| Caso | Qué prueba | Estados | Conflictos |
|---|---|---|---|
| `parentesis` | tabla de referencia del enunciado | 12 | 0 |
| `vacias` | producciones vacías (`A -> epsilon`) | 10 | 0 |
| `prueba1` | producción vacía como alternativa | 4 | 0 |
| `compiladores` | expresiones con `E'`/`T'` anulables | 44 | 0 |
| `postfija` | aumentación automática (`S` recursivo) | 10 | 0 |
| `conflicto_sr` | **shift/reduce**: `E -> E + E \| id` | 5 | 1 |
| `conflicto_rr` | **reduce/reduce**: `A -> a`, `B -> a` | 5 | 1 |
| `declaraciones` | shift/reduce con producciones vacías | 9 | 2 |

`run_tests` verifica además `firstCadena`. `make test` comprueba también que
la herramienta termina con código distinto de cero ante un conflicto. Para
agregar un caso: crear `<caso>.txt` y `<caso>.tabla` y añadir el nombre a la
lista de `test/run_tests.cpp`.

## Compilar con CMake

### macOS

Desde la raíz del repositorio:

```sh
cmake -S ChocoPyScaParser -B ChocoPyScaParser/build-macos
cmake --build ChocoPyScaParser/build-macos
ctest --test-dir ChocoPyScaParser/build-macos --output-on-failure
cmake -E chdir ChocoPyScaParser ./build-macos/chocopy-parser src/chocopy.txt \
  --source test/casesChocopy/01_hola.py
```

### Windows (PowerShell)

Desde la raíz del repositorio:

```powershell
cmake -S ChocoPyScaParser -B ChocoPyScaParser/build-windows
cmake --build ChocoPyScaParser/build-windows --config Debug
ctest --test-dir ChocoPyScaParser/build-windows -C Debug --output-on-failure
Push-Location ChocoPyScaParser
& .\build-windows\Debug\chocopy-parser.exe src\chocopy.txt --source test\casesChocopy\01_hola.py
Pop-Location
```

### Limpiar

Desde la raíz del repositorio, ejecuta:

```sh
cmake -P ChocoPyScaParser/clean_builds.cmake
```

Esto elimina por completo los directorios de build del parser, incluidos los
archivos generados por CMake. El código fuente y las pruebas se conservan.

Los directorios `ChocoPyScaParser/build/` y `ChocoPyScaParser/build-*` están
ignorados por Git.
