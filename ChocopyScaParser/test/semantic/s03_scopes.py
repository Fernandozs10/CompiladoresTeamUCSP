x: int = 1

def externa() -> int:
    total: int = 0
    def interna() -> int:
        nonlocal total
        total = total + 1
        return total
    return interna()

def oculta() -> int:
    x: int = 2
    return x

def lee_global() -> int:
    return x

def sin_nonlocal() -> int:
    n: int = 0
    def hijo() -> int:
        n = 5
        return n
    return hijo()

def mala_global() -> int:
    global noexiste
    return 0

print(externa())
