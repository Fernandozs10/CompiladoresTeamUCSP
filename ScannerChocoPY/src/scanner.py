"""Analizador lexico (scanner) de ChocoPy v2.2, escrito a mano.

Modelo de ejecucion (charla "Scanners" del curso):

  * recorrido caracter a caracter;
  * "maximal munch": se acepta el lexema legal mas largo leyendo de
    izquierda a derecha (Reference 3);
  * encima del reconocedor de tokens hay una capa que maneja la
    estructura de lineas y emite NEWLINE / INDENT / DEDENT
    (Reference 3.1).

En ChocoPy una LINEA LOGICA es exactamente una LINEA FISICA
(Reference 3.1.2: "A logical line is a physical line that contains at
least one token that is not whitespace or comments").  No existe union
de lineas, ni implicita (parentesis/corchetes) ni explicita
(backslash), a diferencia de Python.  Por eso toda linea fisica que no
sea vacia ni solo-comentario produce exactamente un NEWLINE.

El scanner es tolerante a errores: por defecto reporta cada error
lexico y continua (para detectar varios en una sola pasada).  Con
strict=True aborta lanzando LexError en el primero.
"""

from __future__ import annotations

import re

from tokens import KEYWORDS, Token

INT_MAX = 2147483647  # 2**31 - 1  (Reference 3.4.2)

_ID_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*\Z")

# Unicas secuencias de escape validas dentro de string (Reference 3.4.1).
_ESCAPES = {'"': '"', "n": "\n", "t": "\t", "\\": "\\"}

_OP2 = {"//": "INTDIV", "<=": "LE", ">=": "GE", "==": "EQ", "!=": "NE",
        "->": "ARROW"}
_OP1 = {"+": "PLUS", "-": "MINUS", "*": "MULT", "%": "MOD", "<": "LT",
        ">": "GT", "=": "ASSIGN", "(": "LPAREN", ")": "RPAREN",
        "[": "LBRACKET", "]": "RBRACKET", ",": "COMMA", ":": "COLON",
        ".": "DOT"}


class LexError(Exception):
    def __init__(self, message: str, line: int, col: int):
        super().__init__(f"{line}:{col}: error lexico: {message}")
        self.message = message
        self.line = line
        self.col = col


def _is_identifier(s: str) -> bool:
    return bool(s) and _ID_RE.match(s) is not None


def _physical_lines(src: str):
    """Parte la fuente en lineas fisicas aceptando LF, CRLF y CR
    (Reference 3.1.1).  Devuelve pares (numero_de_linea, texto_sin_fin).
    El fin de entrada tambien termina la ultima linea fisica."""
    lines = []
    i, n, start, lineno = 0, len(src), 0, 1
    while i < n:
        c = src[i]
        if c == "\n":
            lines.append((lineno, src[start:i]))
            i += 1
            start, lineno = i, lineno + 1
        elif c == "\r":
            lines.append((lineno, src[start:i]))
            i += 2 if (i + 1 < n and src[i + 1] == "\n") else 1
            start, lineno = i, lineno + 1
        else:
            i += 1
    if start < n:  # ultima linea sin terminador explicito
        lines.append((lineno, src[start:]))
    return lines


class Scanner:
    def __init__(self, source: str, strict: bool = False):
        self.source = source
        self.strict = strict
        self.tokens: list[Token] = []
        self.errors: list[LexError] = []
        self.indents = [0]  # pila de indentacion (Reference 3.1.5)

    # -- utilidades internas ----------------------------------------
    def error(self, msg: str, line: int, col: int) -> None:
        err = LexError(msg, line, col)
        if self.strict:
            raise err
        self.errors.append(err)

    def _emit(self, ttype: str, value, line: int, col: int) -> None:
        self.tokens.append(Token(ttype, value, line, col))

    # -- API publica ----------------------------------------------
    def tokenize(self) -> list[Token]:
        last_lineno = 0

        for lineno, text in _physical_lines(self.source):
            last_lineno = lineno
            n = len(text)

            # Indentacion: tabs expandidos a multiplos de 8 (Reference 3.1.5).
            i = indent = 0
            while i < n and text[i] in " \t":
                indent += 1 if text[i] == " " else 8 - (indent % 8)
                i += 1

            rest = text[i:]
            if rest == "" or rest[0] == "#":
                continue  # linea en blanco o solo comentario (Reference 3.1.3-3.1.4)

            # Es una linea logica (Reference 3.1.2).
            self._handle_indent(indent, lineno)
            self._scan_line(lineno, text, i)
            self._emit("NEWLINE", None, lineno, n + 1)

        # Fin de la entrada: un DEDENT por cada nivel > 0 (Reference 3.1.5).
        eof_line = last_lineno + 1
        while len(self.indents) > 1:
            self.indents.pop()
            self._emit("DEDENT", None, eof_line, 1)
        self._emit("EOF", None, eof_line, 1)
        return self.tokens

    # -- estructura de lineas ------------------------------------
    def _handle_indent(self, indent: int, lineno: int) -> None:
        top = self.indents[-1]
        if indent == top:
            return
        if indent > top:
            self.indents.append(indent)
            self._emit("INDENT", None, lineno, 1)
            return
        while len(self.indents) > 1 and self.indents[-1] > indent:
            self.indents.pop()
            self._emit("DEDENT", None, lineno, 1)
        if self.indents[-1] != indent:
            self.error("nivel de indentacion inconsistente", lineno, 1)
            self.indents.append(indent)  # recuperacion

    # -- reconocimiento de tokens dentro de una linea -----------
    def _scan_line(self, lineno: int, text: str, i: int) -> None:
        n = len(text)
        while i < n:
            c = text[i]
            col = i + 1

            if c in " \t":
                i += 1
                continue
            if c == "#":                       # comentario hasta fin de linea
                return
            if c == '"':
                i = self._scan_string(lineno, text, i)
                continue
            if "0" <= c <= "9":
                i = self._scan_number(lineno, text, i)
                continue
            if c == "_" or "a" <= c <= "z" or "A" <= c <= "Z":
                j = i + 1
                while j < n and (text[j] == "_" or "0" <= text[j] <= "9"
                                 or "a" <= text[j] <= "z"
                                 or "A" <= text[j] <= "Z"):
                    j += 1
                word = text[i:j]
                if word in KEYWORDS:
                    self._emit(word.upper(), None, lineno, col)
                else:
                    self._emit("ID", word, lineno, col)
                i = j
                continue
            i = self._scan_operator(lineno, text, i)

    def _scan_operator(self, lineno: int, text: str, i: int) -> int:
        col = i + 1
        if text[i:i + 2] in _OP2:
            self._emit(_OP2[text[i:i + 2]], None, lineno, col)
            return i + 2
        one = text[i]
        if one in _OP1:
            self._emit(_OP1[one], None, lineno, col)
            return i + 1
        if one == "/":
            self.error("operador '/' no soportado en ChocoPy (usa '//')",
                       lineno, col)
            return i + 1
        if one == "!":
            self.error("'!' inesperado (se esperaba '!=')", lineno, col)
            return i + 1
        self.error(f"caracter ilegal {one!r}", lineno, col)
        return i + 1

    def _scan_number(self, lineno: int, text: str, i: int) -> int:
        n = len(text)
        j = i
        while j < n and "0" <= text[j] <= "9":
            j += 1
        lexeme = text[i:j]
        col = i + 1
        # Maximal munch: se toma la corrida completa de digitos y luego
        # se valida (Reference 3.4.2). El test de longitud va primero para
        # no convertir cadenas de digitos gigantescas.
        in_range = len(lexeme) <= 10 and int(lexeme) <= INT_MAX
        if len(lexeme) > 1 and lexeme[0] == "0":
            self.error(f"literal entero con cero a la izquierda: {lexeme}",
                       lineno, col)
        elif not in_range:
            self.error(
                f"literal entero fuera de rango (maximo {INT_MAX}): {lexeme}",
                lineno, col)
        # Valor de recuperacion: el entero si es representable, si no INT_MAX.
        self._emit("INTEGER", int(lexeme) if in_range else INT_MAX,
                   lineno, col)
        return j

    def _scan_string(self, lineno: int, text: str, i: int) -> int:
        n = len(text)
        col = i + 1
        j = i + 1
        out: list[str] = []
        had_error = False
        while j < n:
            ch = text[j]
            if ch == '"':
                value = "".join(out)
                if not had_error and _is_identifier(value):
                    self._emit("IDSTRING", value, lineno, col)
                else:
                    self._emit("STRING", value, lineno, col)
                return j + 1
            if ch == "\\":
                if j + 1 < n:
                    esc = text[j + 1]
                    if esc in _ESCAPES:
                        out.append(_ESCAPES[esc])
                    else:
                        self.error(f"secuencia de escape ilegal '\\{esc}'",
                                   lineno, j + 1)
                        had_error = True
                        out.append(esc)
                    j += 2
                    continue
                break  # '\' como ultimo caracter: cadena sin terminar
            o = ord(ch)
            if o < 32 or o > 126:
                self.error(
                    f"caracter fuera del rango ASCII 32-126 en string (codigo {o})",
                    lineno, j + 1)
                had_error = True
                j += 1
                continue
            out.append(ch)
            j += 1
        self.error("string literal sin terminar", lineno, col)
        self._emit("STRING", "".join(out), lineno, col)
        return n
