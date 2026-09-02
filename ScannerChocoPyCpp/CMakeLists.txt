cmake_minimum_required(VERSION 3.12)
project(chocopy_scanner_cpp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CHOCOPY_CORE src/tokens.cpp src/scanner.cpp)

add_executable(chocopy-scanner ${CHOCOPY_CORE} src/main.cpp)
add_executable(run_tests ${CHOCOPY_CORE} tests/run_tests.cpp)

if(MSVC)
    target_compile_options(chocopy-scanner PRIVATE /W4 /utf-8)
    target_compile_options(run_tests PRIVATE /W4 /utf-8)
else()
    target_compile_options(chocopy-scanner PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(run_tests PRIVATE -Wall -Wextra -Wpedantic)
    # GCC < 9 necesita enlazar stdc++fs para <filesystem>.
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        target_link_libraries(chocopy-scanner PRIVATE stdc++fs)
        target_link_libraries(run_tests PRIVATE stdc++fs)
    endif()
endif()

enable_testing()
add_test(NAME bateria
         COMMAND run_tests "${CMAKE_CURRENT_SOURCE_DIR}/tests")
