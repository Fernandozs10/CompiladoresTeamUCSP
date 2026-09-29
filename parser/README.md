# Parser ChocoPy C++ — Generador de tablas LR(1)

Herramienta en C++17 que, dada **cualquier** gramática libre de contexto,
genera la colección canónica de ítems LR(1) y las tablas **Action** y
**Goto**, detectando conflictos. No depende de la gramática de ChocoPy ni usa
Bison ni bibliotecas que construyan autómatas o tablas: todo sale del archivo
de entrada.

## Funciones

| Enunciado | Método | Qué hace |
|---|---|---|
| 1. `first()` | `computeFirstSets()` | FIRST de todos los símbolos por punto fijo, con `epsilon` para los anulables |
| 2. `firstCadena(β)` | `computeFirstOfString(beta)` | FIRST de una cadena `s1 … sk` |
| 3. `closure(s)` | `computeClosure(s)` | Cerradura de un conjunto de ítems LR(1) |
| 4. `goto(s, x)` | `computeGoto(s, x)` | Conjunto alcanzado desde `s` al reconocer `x` |
| 5. `coleccionCanonica()` | `buildCanonicalCollection()` | `CC = {cc0, cc1, …}` y el registro de transiciones |
| 6. `llenarTablas()` | `fillTables()` | `Action` y `Goto` con detección de conflictos |

`goto` es palabra reservada de C++, por eso el método se llama `computeGoto`.

Un ítem `[A → β • γ, a]` se representa con **tres enteros** (producción,
posición del punto, índice del terminal de anticipación) y un conjunto de
ítems es un `std::set<Item>`, de modo que dos `cc` se comparan con `==`.
Cada celda de Action guarda sus acciones (`s3`, `r2`, `acc`) junto con los
ítems que las generan; si hay más de una acción distinta, es un conflicto.

## Formato de la gramática

Una producción por línea, con `->` (o `::=`) y alternativas con `|`:

```
Goal -> List
List -> List Pair | Pair
Pair -> ( Pair ) | ( )
```

- Los símbolos van separados por espacios.
- **No terminal**: empieza por mayúscula (`List`, `E'`). **Terminal**: todo
  lo demás (`id`, `(`, `+`, `num`).
- La producción vacía se escribe `epsilon`: `B -> b | epsilon`.
- El **símbolo inicial** es el lado izquierdo de la primera regla.
- `eof` es reservado y lo agrega la herramienta (internamente `$`); no se
  escribe en la gramática.
- **Aumentación**: si el símbolo inicial tiene una sola producción y no
  aparece en ningún lado derecho (como `Goal -> List`), esa regla ya es la
  aumentada y se usa tal cual. Si no, la herramienta agrega `S' -> S`.

## Compilar y ejecutar con `make` (Linux, macOS, MinGW)

Desde `parser/`:

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
cmake -S parser -B parser/build-macos
cmake --build parser/build-macos
ctest --test-dir parser/build-macos --output-on-failure
cmake -E chdir parser/build-macos ./chocopy-parser
```

### Windows (PowerShell)

Desde la raíz del repositorio:

```powershell
cmake -S parser -B parser/build-windows
cmake --build parser/build-windows --config Debug
ctest --test-dir parser/build-windows -C Debug --output-on-failure
Push-Location parser/build-windows
& .\Debug\chocopy-parser.exe
Pop-Location
```

### Limpiar

Desde la raíz del repositorio, ejecuta:

```sh
cmake -P parser/clean_builds.cmake
```

Esto elimina por completo los directorios de build del parser, incluidos los
archivos generados por CMake. El código fuente y las pruebas se conservan.

Los directorios `parser/build/` y `parser/build-*` están ignorados por Git.
