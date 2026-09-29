# Parser ChocoPy C++

Requiere CMake 3.12 o posterior y un compilador de C++17. CMake selecciona el
generador disponible en cada sistema; usa `cmake --build` para compilar tanto
en macOS como en Windows.

## macOS

Desde la raíz del repositorio:

```sh
cmake -S parser -B parser/build-macos
cmake --build parser/build-macos
ctest --test-dir parser/build-macos --output-on-failure
cmake -E chdir parser/build-macos ./chocopy-parser
```

## Windows (PowerShell)

Desde la raíz del repositorio:

```powershell
cmake -S parser -B parser/build-windows
cmake --build parser/build-windows --config Debug
ctest --test-dir parser/build-windows -C Debug --output-on-failure
Push-Location parser/build-windows
& .\Debug\chocopy-parser.exe
Pop-Location
```

## Limpiar

Desde la raíz del repositorio, ejecuta:

```sh
cmake -P parser/clean_builds.cmake
```

Esto elimina por completo los directorios de build del parser, incluidos los
archivos generados por CMake. El código fuente y las pruebas se conservan.

Los directorios `parser/build/` y `parser/build-*` están ignorados por Git.