# Repositorio Central — Grupo de Compiladores

Repositorio central del **Grupo de Compiladores** de la Universidad Católica
San Pablo (UCSP). Aquí se concentra todo el trabajo del curso: el **proyecto
final** (un compilador para el lenguaje **ChocoPy v2.2**) desarrollado de forma
incremental a lo largo del semestre, junto con las **tareas, ejercicios y
proyectos extra** que se van pidiendo en clase.

> Curso de Compiladores · Docente: MSc. Gina Muñoz Salas

---

## Integrantes

| Nombre | Rol en el equipo |
|---|---|
| **Eduardo Jacob Cruz Laura** | Desarrollo · revisión |
| **Alexander Carpio Mamani** | Desarrollo · revisión |
| **Fernando Abraham Zegarra Maratuech** | Desarrollo · mantenimiento del repo |

Todos los integrantes participan en diseño, implementación y pruebas. Los roles
son orientativos y rotan según la fase del proyecto.

---

## Estructura del repositorio

```
CompiladoresTeamUCSP/
├── README.md                 # este archivo
├── Recursos/                  # material de referencia del curso
│   ├── chocopy_language_reference.(md|pdf)   # manual de ChocoPy v2.2
│   └── Compilers (10).(md|pdf)               # diapositivas "Scanners"
├── ScannerChocoPY/            # Tarea 1 — analizador léxico de ChocoPy (Python)
├── Ejercicios/                # ejercicios cortos de clase
│   └── Ejercicio 1 - LL(1)/
└── ProyectoFinal/            # (en construcción) compilador completo de ChocoPy
```

Cada tarea o proyecto vive en su **propia carpeta** con su **README** y sus
instrucciones de compilación/ejecución. Este README solo da la vista general.

---

## Proyecto final: compilador de ChocoPy

**ChocoPy** es un dialecto de Python 3.6 con tipado estático, diseñado en
UC Berkeley para poder implementar un compilador completo en un semestre
(ver `Recursos/chocopy_language_reference.pdf`).

El compilador se construye por fases. Estado previsto:

| Fase | Descripción | Estado | Carpeta |
|---|---|---|---|
| 1. Análisis léxico | Fuente → flujo de tokens (`NEWLINE`/`INDENT`/`DEDENT`, literales, operadores) | ✅ Completo | [`ScannerChocoPY/`](ScannerChocoPY/) |
| 2. Análisis sintáctico | Tokens → AST según la gramática (Figura 3 del manual) | 🔜 Pendiente | `ProyectoFinal/parser/` |
| 3. Análisis semántico | Chequeo de tipos y de alcance (Sección 5 del manual) | ⏳ Planificado | `ProyectoFinal/semantic/` |
| 4. Generación de código | Emisión de código objetivo + semántica operacional (Sección 6) | ⏳ Planificado | `ProyectoFinal/codegen/` |
| 5. Runtime y pruebas E2E | Soporte de ejecución y batería de programas completos | ⏳ Planificado | `ProyectoFinal/tests/` |

> La leyenda se actualiza conforme avanza el curso.

---

## Tareas y proyectos extra

### Tarea 1 — Scanner de ChocoPy

Analizador léxico de ChocoPy v2.2 escrito **en Python puro y a mano** (sin
generadores tipo `lex`/`flex`/JFlex). Implementa la Sección 3 del manual:
estructura de líneas con pila de indentación, identificadores y las 35
palabras clave, literales enteros (con control de rango de 32 bits) y de
cadena (`STRING`/`IDSTRING`), los 20 operadores y delimitadores, y detección
de errores léxicos con modo tolerante y modo `--strict`.

- Código y documentación: [`ScannerChocoPY/README.md`](ScannerChocoPY/README.md)
- Batería de pruebas: `python ScannerChocoPY/tests/run_tests.py` (21 casos)

### Ejercicios de clase

Ejercicios cortos resueltos durante las sesiones (análisis LL(1), FIRST/FOLLOW,
autómatas, etc.). Ver [`Ejercicios/`](Ejercicios/).

---

## Recursos

| Archivo | Contenido |
|---|---|
| `Recursos/chocopy_language_reference.pdf` | Manual y referencia oficial de ChocoPy v2.2 (léxico, gramática, tipos, semántica) |
| `Recursos/Compilers (10).pdf` | Diapositivas del curso sobre *Scanners* (RE → NFA → DFA, maximal munch, implementación) |

Bibliografía base del curso: *Engineering a Compiler* (Cooper & Torczon, 3.ª ed.)
y *Compilers: Principles, Techniques, and Tools* (Aho, Lam, Sethi & Ullman).

---

## Cómo empezar

```bash
git clone https://github.com/Fernandozs10/CompiladoresTeamUCSP.git
cd CompiladoresTeamUCSP
```

Requisitos generales:

- **Python 3.8+** para el scanner y (previsiblemente) las siguientes fases.
- No hay dependencias externas por ahora; cada carpeta declara las suyas si
  las tuviera.

Para correr las pruebas del scanner:

```bash
python ScannerChocoPY/tests/run_tests.py
```

---

## Convenciones de trabajo

**Ramas**

- `main` — siempre estable; solo recibe cambios vía Pull Request.
- `feature/<tema>` — trabajo en curso (ej. `feature/parser-expresiones`).
- `fix/<tema>` — correcciones puntuales.

**Commits**

- Mensajes en español, en imperativo y concretos:
  `Agrega manejo de DEDENT al final del archivo`.
- Commits pequeños y con un solo propósito.

**Pull Requests**

- Un PR por tarea o por fase; describir qué se hizo y cómo probarlo.
- Al menos **una revisión** de otro integrante antes de fusionar.
- No fusionar con la batería de pruebas en rojo.

**Estructura de una tarea nueva**

1. Carpeta propia en la raíz (`NombreTarea/`).
2. `README.md` con enunciado, decisiones de diseño e instrucciones.
3. Pruebas reproducibles (`tests/` o equivalente).

---

## Estado del repositorio

- [x] Estructura inicial y recursos del curso
- [x] Tarea 1 — Scanner de ChocoPy (21/21 pruebas)
- [ ] Tarea 2 — (por asignar)
- [ ] Proyecto final — Parser
- [ ] Proyecto final — Análisis semántico
- [ ] Proyecto final — Generación de código
