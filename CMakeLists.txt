
cmake_minimum_required(VERSION 3.10)
project(traceroute C)

# Genera build fuori albero
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# Specifica standard C
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Directory include
include_directories(${PROJECT_SOURCE_DIR}/include)

# Aggiungi sorgenti
file(GLOB_RECURSE SOURCES src/*.c)

# Target eseguibile
add_executable(traceroute ${SOURCES})

