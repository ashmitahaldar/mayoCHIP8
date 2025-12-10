cmake_minimum_required(VERSION 3.16)
project(mayoCHIP8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SDL2 is installed via Homebrew on macOS and exposes a CMake config package.
# If CMake cannot find SDL2, set SDL2_DIR to the SDL2Config.cmake directory,
# e.g. -DSDL2_DIR=/opt/homebrew/lib/cmake/SDL2
find_package(SDL2 REQUIRED CONFIG)

add_executable(mayochip8
    src/main.cpp
    src/chip8.cpp
    src/platform.cpp
)

target_include_directories(mayochip8 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(mayochip8 PRIVATE SDL2::SDL2)
