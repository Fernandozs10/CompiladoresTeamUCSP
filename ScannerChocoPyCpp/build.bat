@echo off
REM Compilacion con MSVC (Visual Studio Build Tools) en Windows.
REM
REM   build.bat          compila chocopy-scanner.exe y run_tests.exe en bin\
REM   build.bat test     compila y ejecuta la bateria de pruebas
REM
REM Si "cl" no esta en el PATH, el script busca vcvarsall.bat en las rutas
REM de instalacion habituales de Visual Studio. Como alternativa, abre el
REM "Developer Command Prompt for VS" y vuelve a ejecutar.
REM
REM Nota: algunas instalaciones de VS imprimen un aviso sobre "vswhere.exe"
REM al ejecutar vcvarsall.bat. Es de Microsoft, no de este script, y no
REM afecta a la compilacion.

setlocal enabledelayedexpansion

cd /d "%~dp0"

where cl >nul 2>&1
if not errorlevel 1 goto :compilar

set "VCVARS="

REM Guardamos las dos raices en variables sin parentesis: "%ProgramFiles(x86)%"
REM rompe los bloques if(...) / for(...) de batch por su ")".
set "PF64=%ProgramFiles%"
set "PF32=%ProgramFiles(x86)%"
if not defined PF32 set "PF32=%PF64%"

REM Buscar vcvarsall.bat en las rutas de instalacion habituales.
for %%e in (2022 2019 2017) do for %%v in (BuildTools Community Professional Enterprise) do (
    if not defined VCVARS (
        if exist "!PF64!\Microsoft Visual Studio\%%e\%%v\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=!PF64!\Microsoft Visual Studio\%%e\%%v\VC\Auxiliary\Build\vcvarsall.bat"
    )
    if not defined VCVARS (
        if exist "!PF32!\Microsoft Visual Studio\%%e\%%v\VC\Auxiliary\Build\vcvarsall.bat" set "VCVARS=!PF32!\Microsoft Visual Studio\%%e\%%v\VC\Auxiliary\Build\vcvarsall.bat"
    )
)

if not defined VCVARS (
    echo No se encontro cl.exe ni vcvarsall.bat.
    echo Abre el "Developer Command Prompt for VS" y vuelve a ejecutar.
    exit /b 1
)

echo Usando !VCVARS!
call "!VCVARS!" x64 >nul
if errorlevel 1 exit /b 1

:compilar
if not exist bin mkdir bin

cl /nologo /std:c++17 /EHsc /W4 /utf-8 /O2 ^
   src\tokens.cpp src\scanner.cpp src\main.cpp ^
   /Fe:bin\chocopy-scanner.exe /Fo:bin\
if errorlevel 1 exit /b 1

cl /nologo /std:c++17 /EHsc /W4 /utf-8 /O2 ^
   src\tokens.cpp src\scanner.cpp tests\run_tests.cpp ^
   /Fe:bin\run_tests.exe /Fo:bin\
if errorlevel 1 exit /b 1

echo.
echo Compilado: bin\chocopy-scanner.exe y bin\run_tests.exe

if /I "%~1"=="test" (
    echo.
    bin\run_tests.exe tests
    exit /b !errorlevel!
)

endlocal
