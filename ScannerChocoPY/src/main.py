"""CLI del scanner de ChocoPy.

Uso:
    python src/main.py programa.py            # tokens en texto
    python src/main.py programa.py --json     # tokens en JSON
    python src/main.py programa.py --strict   # abortar al primer error
"""

from __future__ import annotations

import argparse
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from scanner import LexError, Scanner          # noqa: E402
from tokens import render                       # noqa: E402


def main(argv=None) -> int:
    p = argparse.ArgumentParser(
        description="Analizador lexico (scanner) de ChocoPy v2.2")
    p.add_argument("archivo", help="archivo fuente de ChocoPy")
    p.add_argument("--json", action="store_true",
                   help="emitir los tokens en formato JSON")
    p.add_argument("--strict", action="store_true",
                   help="detenerse en el primer error lexico")
    args = p.parse_args(argv)

    with open(args.archivo, encoding="utf-8", errors="replace", newline="") as f:
        source = f.read()

    sc = Scanner(source, strict=args.strict)
    try:
        tokens = sc.tokenize()
    except LexError as e:
        print(e, file=sys.stderr)
        return 2

    if args.json:
        print(json.dumps(
            [{"type": t.type, "value": t.value, "line": t.line, "col": t.col}
             for t in tokens],
            ensure_ascii=False, indent=2))
    else:
        print(render(tokens, sc.errors))

    return 1 if sc.errors else 0


if __name__ == "__main__":
    sys.exit(main())
