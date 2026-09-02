# Scanner de ChocoPy v2.2 — versión en C++

Analizador léxico (scanner) para el lenguaje **ChocoPy v2.2**, escrito
**en C++17 puro y a mano** (sin generadores tipo `lex`/`flex`/JFlex).
Implementa la Sección 3 ("Lexical structure") del
*ChocoPy Language Manual and Reference*.

Es un **port 1:1 de la versión en Python** ([`../ScannerChocoPY/`](../ScannerChocoPY/)):
misma arquitectura, mismos nombres de tokens, mismos mensajes de error y
**la misma salida byte a byte**. Ambas implementaciones se validan contra
la *misma* batería de archivos `.tokens`.

## Requisitos

- Un compilador con soporte de **C++17** (necesita `<filesystem>`):
  GCC 8+, Clang 7+ o MSVC 2017 15.7+.
- Sin dependencias externas: solo la biblioteca estándar.
- Opcionalmente **CMake 3.12+**, si prefieres esa ruta.

## Estructura del proyecto

```
ScannerChocoPyCpp/
├── README.md
├── Makefile              # compilación con g++/clang++ (Linux, macOS, MinGW)
├── build.bat             # compilación con MSVC en Windows
├── CMakeLists.txt        # alternativa multiplataforma
├── src/
│   ├── tokens.h/.cpp     # Token, palabras clave, operadores, UTF-8, formateo
│   ├── scanner.h/.cpp    # el scanner: reconocedor + estructura de líneas
│   └── main.cpp          # interfaz de línea de comandos
└── tests/
    ├── run_tests.cpp     # ejecuta la batería y compara con la salida esperada
    ├── cases/            # programas ChocoPy válidos + su salida esperada (.tokens)
    └── errores/          # programas con errores léxicos + su salida esperada
```

## Compilación

### Opción A — g++ / clang++ (Linux, macOS, MinGW)

```bash
make
```

Genera `bin/chocopy-scanner` y `bin/run_tests`. Para compilar y probar de
una vez:

```bash
make test
```

> Con GCC 8 hay que enlazar el sistema de archivos aparte:
> `make FSLIB=-lstdc++fs`

Compilación manual equivalente, si no quieres usar `make`:

```bash
g++ -std=c++17 -O2 -Wall -Wextra src/tokens.cpp src/scanner.cpp src/main.cpp -o chocopy-scanner
```

### Opción B — MSVC (Windows)

Desde un *Developer Command Prompt for VS* (o dejando que el script
localice `vcvarsall.bat`):

```bash
build.bat
```

Genera `bin\chocopy-scanner.exe` y `bin\run_tests.exe`. Para compilar y
probar:

```bash
build.bat test
```

Compilación manual equivalente:

```bash
cl /nologo /std:c++17 /EHsc /utf-8 /O2 src\tokens.cpp src\scanner.cpp src\main.cpp /Fe:chocopy-scanner.exe
```

### Opción C — CMake (multiplataforma)

```bash
cmake -S . -B build && cmake --build build
```

```bash
ctest --test-dir build --output-on-failure
```

## Uso

```bash
./bin/chocopy-scanner <archivo>            # imprime la lista de tokens (texto)
./bin/chocopy-scanner <archivo> --json     # imprime los tokens en formato JSON
./bin/chocopy-scanner <archivo> --strict   # se detiene en el primer error léxico
```

Ejemplo:

```bash
./bin/chocopy-scanner tests/cases/12_clase.py
```

Código de salida: `0` si no hubo errores léxicos, `1` si hubo errores
(modo tolerante), `2` si abortó en modo `--strict` o si los argumentos
son inválidos.

### Formato de salida (texto)

Una línea por token, separada por tabuladores:
`linea:columna` (1-indexado), tipo de token y, si aplica, su valor.

```
1:1     CLASS
1:7     ID      animal
1:13    LPAREN
...
2:1     INDENT
...
15:1    DEDENT
15:1    DEDENT
15:1    EOF
```

Los errores léxicos se listan **al final** de la salida, como
`ERROR  linea:columna  mensaje`.

## Tokens que reconoce

| Grupo | Tipos de token |
|---|---|
| Estructura de línea (3.1) | `NEWLINE`, `INDENT`, `DEDENT`, `EOF` |
| Identificadores (3.2) | `ID` |
| Palabras clave (3.3) | las 35 de Python: `FALSE NONE TRUE AND AS ASSERT ASYNC AWAIT BREAK CLASS CONTINUE DEF DEL ELIF ELSE EXCEPT FINALLY FOR FROM GLOBAL IF IMPORT IN IS LAMBDA NONLOCAL NOT OR PASS RAISE RETURN TRY WHILE WITH YIELD` |
| Literales (3.4) | `INTEGER`, `STRING`, `IDSTRING` |
| Operadores y delimitadores (3.5) | `PLUS +`, `MINUS -`, `MULT *`, `INTDIV //`, `MOD %`, `LT <`, `GT >`, `LE <=`, `GE >=`, `EQ ==`, `NE !=`, `ASSIGN =`, `LPAREN (`, `RPAREN )`, `LBRACKET [`, `RBRACKET ]`, `COMMA ,`, `COLON :`, `DOT .`, `ARROW ->` |

`IDSTRING` es un literal de cadena cuyo contenido tiene la forma de un
identificador (p. ej. `"animal"`, `"int"`); el resto son `STRING`
(Reference 3.4.1). El valor de `INTEGER` es el entero ya interpretado
en base 10; el de `STRING`/`IDSTRING` es el texto entre comillas con
los escapes ya aplicados.

## Batería de pruebas

```bash
./bin/run_tests               # ejecuta las 21 pruebas
./bin/run_tests --generate    # regenera los archivos .tokens esperados
./bin/run_tests <ruta/tests>  # usa otra carpeta de casos
```

Cada caso es un par `nombre.py` (entrada) + `nombre.tokens` (salida
esperada). `run_tests` corre el scanner sobre cada entrada, compara la
salida completa (tokens + errores) contra el `.tokens` y muestra
`PASS`/`FAIL` con un *diff* unificado en caso de fallo. El código de
salida es `0` solo si pasan todas.

Los `.tokens` son **exactamente los mismos archivos** que usa la versión
en Python, así que sirven de prueba cruzada entre implementaciones.

### Qué cubre la batería

**`tests/cases/` — programas válidos:**

| Archivo | Qué ejercita |
|---|---|
| `01_hola.py` | tokens básicos, `STRING`, `NEWLINE`, `EOF` |
| `02_asignacion.py` | definición de variables con anotación de tipo |
| `03_indentacion.py` | `INDENT`/`DEDENT`, bloques anidados, `else` |
| `04_dedent_multiple.py` | un solo cambio de sangría cierra varios bloques a la vez |
| `05_asignacion_multiple.py` | asignación encadenada `x = y = z = 1` (varios `=`) |
| `06_lineas_blanco_comentarios.py` | líneas en blanco y comentarios `#` ignorados (sin `NEWLINE`) |
| `07_operadores.py` | los 20 operadores y delimitadores |
| `08_strings_escapes.py` | escapes `\"`, `\\`, `\n`, `\t` |
| `09_idstring_vs_string.py` | `IDSTRING` vs `STRING`, cadena vacía |
| `10_enteros.py` | `0`, un valor normal y el máximo `2147483647` |
| `11_keywords_vs_id.py` | regla de match más largo (`classic` es `ID`, no `class`) |
| `12_clase.py` | programa completo con clases, métodos y herencia |
| `13_sin_union_de_lineas.py` | ChocoPy **no** une líneas: un `[` abierto no suprime el `NEWLINE` (3.1.2) |

**`tests/errores/` — errores léxicos:**

| Archivo | Error detectado |
|---|---|
| `e01_caracter_ilegal.py` | símbolo `$` fuera del alfabeto |
| `e02_string_sin_cerrar.py` | cadena sin comilla de cierre |
| `e03_escape_ilegal.py` | secuencia de escape `\o` no reconocida |
| `e04_entero_cero_izquierda.py` | entero `007` con cero a la izquierda |
| `e05_entero_fuera_de_rango.py` | entero mayor que 2³¹−1 |
| `e06_slash_simple.py` | operador `/` (no existe en ChocoPy; solo `//`) |
| `e07_indentacion_inconsistente.py` | sangría que no coincide con ningún nivel de la pila |
| `e08_no_ascii_en_string.py` | carácter fuera del rango ASCII 32–126 en un `STRING` |

## Detalles de implementación

- **Modelo de ejecución**: recorrido carácter a carácter con la regla
  de *maximal munch* (se acepta el lexema legal más largo leyendo de
  izquierda a derecha).

- **Estructura de líneas** (Reference 3.1). Una capa sobre el
  reconocedor de tokens:
  - mantiene una **pila de indentación** que empieza en `[0]`;
  - expande tabs a espacios hasta el siguiente múltiplo de 8
    (Reference 3.1.5);
  - acepta finales de línea `LF`, `CRLF` y `CR` (Reference 3.1.1);
  - ignora por completo las líneas en blanco y las de solo comentario
    `#`: no generan `NEWLINE` ni afectan la indentación
    (Reference 3.1.3–3.1.4);
  - en ChocoPy **una línea lógica es exactamente una línea física**
    (Reference 3.1.2): **no hay unión de líneas**, ni implícita
    (paréntesis/corchetes) ni explícita (backslash). Por eso toda
    línea física no vacía produce exactamente un `NEWLINE`;
  - al final de la entrada emite un `DEDENT` por cada nivel de
    indentación mayor que 0 y luego `EOF` (Reference 3.1.5).

- **Literales enteros** (Reference 3.4.2): se consume la corrida
  completa de dígitos (maximal munch) y luego se valida. Se marca
  como error el cero a la izquierda (`007`) y el valor mayor que
  2³¹−1. En ambos casos se emite igualmente un token `INTEGER` con un
  valor de recuperación, para poder seguir analizando el resto.

- **Literales de cadena** (Reference 3.4.1): solo caracteres ASCII
  32–126; escapes válidos `\"`, `\\`, `\n`, `\t` (cualquier otro es
  error). Se clasifica como `IDSTRING` si el contenido tiene forma de
  identificador, `STRING` en caso contrario (si hubo algún error de
  cadena siempre se emite `STRING`).

- **Manejo de errores**: por defecto el scanner **reporta y continúa**,
  aplicando una recuperación local (saltar el carácter, cerrar la
  cadena al final de la línea, etc.) para poder detectar varios
  errores en una sola pasada. Con `--strict` lanza `LexError` en el
  primero. Los errores estrictamente sintácticos (paréntesis
  desbalanceados, `INDENT` sin sentido, etc.) **no** son competencia
  del scanner y se dejan para el parser.

## Decisiones propias del port a C++

Estas son las diferencias de *implementación* (no de comportamiento)
respecto de la versión en Python:

- **Se trabaja sobre puntos de código, no sobre bytes.** Python opera
  sobre `str` (secuencia de code points). Para que las columnas y los
  códigos de carácter de los mensajes de error coincidan, la entrada se
  decodifica de UTF-8 a `std::u32string` (`utf8Decode`), con las
  secuencias inválidas sustituidas por U+FFFD igual que hace Python con
  `errors="replace"`. Si se trabajara sobre `char`, una `é` contaría
  como dos columnas y produciría dos errores en vez de uno.

- **`pyRepr()` reproduce `repr()` de Python** para mostrar el valor de
  `STRING`/`IDSTRING` y del carácter ilegal: elige comilla simple salvo
  que la cadena contenga `'` y no `"`, escapa `\\`, `\n`, `\t`, `\r`, y
  usa `\xNN` / `\uXXXX` / `\UXXXXXXXX` para los caracteres no
  imprimibles (controles, separadores, formato como el BOM, sustitutos
  y uso privado). No se cubre la categoría Cn (code points no
  asignados), que exigiría las tablas Unicode completas.

- **`stdout` se deja en modo texto.** En Windows la CRT traduce
  `\n` → `\r\n`, exactamente igual que `print()` de Python, de modo que
  la salida redirigida a un archivo es byte a byte idéntica en ambas
  implementaciones. Lo mismo aplica al `--generate` de `run_tests`.

- **El valor del token** se guarda en un `struct` con una etiqueta
  `ValueKind` (`None`/`Int`/`Str`) en lugar del tipado dinámico de
  Python.

- **`--json`** reproduce el formato de
  `json.dumps(..., ensure_ascii=False, indent=2)`.

## Verificación cruzada con la versión en Python

Ambos scanners producen **la misma salida byte a byte**. Se comprobó
sobre todo el corpus de pruebas, sobre casos límite (archivo vacío, solo
espacios, `CRLF` + tabuladores, enteros de 30 dígitos, BOM, comillas
mezcladas, caracteres no ASCII) y con entradas aleatorias, comparando
`stdout`, `stderr` y el código de salida en los tres modos
(texto, `--json`, `--strict`).

En Linux/macOS:

```bash
for f in tests/cases/*.py tests/errores/*.py; do
  diff <(./bin/chocopy-scanner "$f") <(python3 ../ScannerChocoPY/src/main.py "$f") || echo "DIFIERE: $f"
done
```
