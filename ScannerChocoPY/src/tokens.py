"""Definiciones de tokens para el scanner de ChocoPy v2.2.

La lista de categorias de tokens sigue la Seccion 3 del
"ChocoPy Language Manual and Reference" (v2.2).
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any

# --- Palabras clave (Reference 3.3) --------------------------------------
# ChocoPy reutiliza EXACTAMENTE la lista de palabras clave de Python 3.6,
# aunque varias (async, await, lambda, ...) no aparezcan en la gramatica.
KEYWORDS = {
    "False", "None", "True", "and", "as", "assert", "async", "await",
    "break", "class", "continue", "def", "del", "elif", "else", "except",
    "finally", "for", "from", "global", "if", "import", "in", "is",
    "lambda", "nonlocal", "not", "or", "pass", "raise", "return", "try",
    "while", "with", "yield",
}

# --- Operadores y delimitadores (Reference 3.5) -------------------------
# Nombre simbolico -> lexema.
OPERATORS = {
    "PLUS": "+", "MINUS": "-", "MULT": "*", "INTDIV": "//", "MOD": "%",
    "LT": "<", "GT": ">", "LE": "<=", "GE": ">=", "EQ": "==", "NE": "!=",
    "ASSIGN": "=", "LPAREN": "(", "RPAREN": ")", "LBRACKET": "[",
    "RBRACKET": "]", "COMMA": ",", "COLON": ":", "DOT": ".", "ARROW": "->",
}

# --- Tokens de estructura de linea (Reference 3.1) ---------------------
STRUCTURE = ("NEWLINE", "INDENT", "DEDENT", "EOF")


@dataclass
class Token:
    """Un token: <categoria, lexema> con su posicion (1-indexada)."""

    type: str
    value: Any
    line: int
    col: int

    def __str__(self) -> str:
        if self.value is None:
            shown = ""
        elif self.type in ("STRING", "IDSTRING"):
            shown = repr(self.value)
        else:
            shown = str(self.value)
        return f"{self.line}:{self.col}\t{self.type}\t{shown}".rstrip()


def render(tokens, errors) -> str:
    """Formato de texto determinista: primero los tokens, luego los errores.

    Lo usan tanto el CLI (main.py) como la bateria de pruebas.
    """
    out = [str(t) for t in tokens]
    for e in errors:
        out.append(f"ERROR\t{e.line}:{e.col}\t{e.message}")
    return "\n".join(out)
