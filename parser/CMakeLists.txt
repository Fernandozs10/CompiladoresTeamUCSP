cmake_minimum_required(VERSION 3.12)
project(chocopy_parser_cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(parser_core src/parser.cpp)
target_include_directories(parser_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

if(MSVC)
    target_compile_options(parser_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(parser_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(chocopy-parser src/main.cpp)
target_link_libraries(chocopy-parser PRIVATE parser_core)
configure_file(src/compiladores.txt compiladores.txt COPYONLY)

enable_testing()
add_executable(parser_unit_tests test/run_tests.cpp)
target_link_libraries(parser_unit_tests PRIVATE parser_core)
add_test(NAME parser_tests
         COMMAND parser_unit_tests "${CMAKE_CURRENT_SOURCE_DIR}/test/cases/prueba1.txt")
