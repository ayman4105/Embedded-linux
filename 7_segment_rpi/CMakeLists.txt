# Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# Project name
project(IOStreamProject)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# List of source files
set(SOURCES
    src/main.cpp
    src/IOStream.cpp
    src/IStream.cpp
    src/OStream.cpp
    src/stream.cpp
    src/SevenSegment.cpp
    src/gpio_27.cpp
)

# List of header files
set(HEADERS
    inc/IOStream.hpp
    inc/IStream.hpp
    inc/OStream.hpp
    inc/stream.hpp
    inc/SevenSegment.hpp
    inc/gpio_27.hpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
