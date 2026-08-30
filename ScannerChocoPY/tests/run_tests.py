"""Bateria de pruebas del scanner de ChocoPy.

Recorre tests/cases/ y tests/errores/, ejecuta el scanner sobre cada
archivo .py y compara la salida (tokens + errores) contra el archivo
.tokens de referencia con el mismo nombre.

    python tests/run_tests.py              # ejecutar la bateria
    python tests/run_tests.py --generate   # (re)generar los .tokens esperados
"""

from __future__ import annotations

import difflib
import os
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(ROOT, "..", "src"))

from scanner import Scanner          # noqa: E402
from tokens import render            # noqa: E402

CASE_DIRS = [os.path.join(ROOT, "cases"), os.path.join(ROOT, "errores")]


def scan_file(path: str) -> str:
    with open(path, encoding="utf-8", errors="replace", newline="") as f:
        source = f.read()
    sc = Scanner(source)
    tokens = sc.tokenize()
    return render(tokens, sc.errors)


def iter_cases():
    for d in CASE_DIRS:
        for name in sorted(os.listdir(d)):
            if name.endswith(".py"):
                yield os.path.join(d, name)


def generate() -> None:
    for py in iter_cases():
        out = scan_file(py)
        with open(py[:-3] + ".tokens", "w", encoding="utf-8") as f:
            f.write(out + "\n")
        print("escrito", os.path.basename(py) + ".tokens")


def run() -> int:
    total = passed = 0
    for py in iter_cases():
        total += 1
        name = os.path.basename(py)
        actual = scan_file(py).rstrip("\n")
        exp_path = py[:-3] + ".tokens"
        if not os.path.exists(exp_path):
            print(f"FAIL  {name}  (falta {name[:-3]}.tokens)")
            continue
        expected = open(exp_path, encoding="utf-8").read().rstrip("\n")
        if actual == expected:
            passed += 1
            print(f"PASS  {name}")
        else:
            print(f"FAIL  {name}")
            for line in difflib.unified_diff(
                    expected.splitlines(), actual.splitlines(),
                    "esperado", "obtenido", lineterm=""):
                print("    " + line)
    print(f"\n{passed}/{total} pruebas superadas")
    return 0 if passed == total else 1


if __name__ == "__main__":
    if "--generate" in sys.argv:
        generate()
    else:
        sys.exit(run())
