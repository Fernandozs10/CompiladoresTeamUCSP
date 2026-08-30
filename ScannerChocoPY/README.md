# Scanner de ChocoPy v2.2

Analizador léxico (scanner) para el lenguaje **ChocoPy v2.2**, Python puro (sin generadores tipo `lex`/`flex`/JFlex).
Implementa la Sección 3 ("Lexical structure") del
*ChocoPy Language Manual and Reference*.

## Requisitos

- **Python 3.8 o superior**. Sin dependencias externas: solo la
  librería estándar.

No hay paso de compilación (es Python interpretado).

## Estructura del proyecto

```
ScannerChocoPY/
├── README.md
├── src/
│   ├── tokens.py     # clase Token, palabras clave, operadores, formateo
│   ├── scanner.py    # el scanner: reconocedor de tokens + estructura de líneas
│   └── main.py       # interfaz de línea de comandos
└── tests/
    ├── run_tests.py  # ejecuta toda la batería y compara con la salida esperada
    ├── cases/        # programas ChocoPy válidos + su salida esperada (.tokens)
    └── errores/      # programas con errores léxicos + su salida esperada
```

## Uso

Desde la carpeta `ScannerChocoPY/`:

```bash
python src/main.py <archivo>            # imprime la lista de tokens (texto)
python src/main.py <archivo> --json     # imprime los tokens en formato JSON
python src/main.py <archivo> --strict   # se detiene en el primer error léxico
```

Ejemplo:

```bash
python src/main.py tests/cases/12_clase.py
```

Código de salida: `0` si no hubo errores léxicos, `1` si hubo errores
(modo tolerante), `2` si abortó en modo `--strict`.

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
python tests/run_tests.py              # ejecuta las 21 pruebas
python tests/run_tests.py --generate   # regenera los archivos .tokens esperados
```

Cada caso es un par `nombre.py` (entrada) + `nombre.tokens` (salida
esperada). `run_tests.py` corre el scanner sobre cada entrada, compara
la salida completa (tokens + errores) contra el `.tokens` y muestra
`PASS`/`FAIL` con un *diff* unificado en caso de fallo. El código de
salida es `0` solo si pasan todas.

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
  2³¹−1. En ambos casos se emite igualmente un token `INTEGER` con el
  valor interpretado, para poder seguir analizando el resto.

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
