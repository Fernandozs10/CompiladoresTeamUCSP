cmake_minimum_required(VERSION 3.12)
project(chocopy_parser_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(parser_core src/parser.cpp src/panic_recovery.h)
target_include_directories(parser_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(scanner_core src/scanner.cpp src/tokens.cpp)
target_include_directories(scanner_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(MSVC)
    target_compile_options(parser_core PRIVATE /W4 /utf-8)
    target_compile_options(scanner_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(parser_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(scanner_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(chocopy-parser src/main.cpp)
target_link_libraries(chocopy-parser PRIVATE parser_core scanner_core)
configure_file(src/compiladores.txt compiladores.txt COPYONLY)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src")
configure_file(src/chocopy.txt src/chocopy.txt COPYONLY)

enable_testing()
add_executable(parser_unit_tests test/run_tests.cpp)
target_link_libraries(parser_unit_tests PRIVATE parser_core)
add_test(NAME parser_tests
         COMMAND parser_unit_tests "${CMAKE_CURRENT_SOURCE_DIR}/test/cases")
add_test(NAME chocopy_scan_parse_assignments
         COMMAND chocopy-parser "${CMAKE_CURRENT_SOURCE_DIR}/src/chocopy.txt"
                 --source "${CMAKE_CURRENT_SOURCE_DIR}/test/casesChocopy/02_asignacion.py")
add_test(NAME chocopy_scan_parse_class
         COMMAND chocopy-parser "${CMAKE_CURRENT_SOURCE_DIR}/src/chocopy.txt"
                 --source "${CMAKE_CURRENT_SOURCE_DIR}/test/casesChocopy/12_clase.py")
