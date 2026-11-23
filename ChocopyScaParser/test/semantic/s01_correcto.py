x: int = 1
nombres: [str] = None
n: int = 0

class Animal(object):
    edad: int = 0

    def crecer(self: Animal, anios: int) -> int:
        self.edad = self.edad + anios
        return self.edad

def f(y: int) -> int:
    x: int = 2
    def g() -> int:
        return x + y
    return g()

def contar() -> int:
    global x
    x = x + 1
    return x

a: Animal = None
a = Animal()
print(f(3))
print(a.crecer(2))
if x > 0:
    print("positivo")
elif x == 0:
    print("cero")
else:
    pass
while x < 5:
    x = x + 1
for n in [1, 2, 3]:
    print(n)
