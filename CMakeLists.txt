cmake_minimum_required(VERSION 3.5)
project(unnamed_game VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(unnamed_game main.cpp)
target_link_libraries(unnamed_game PRIVATE SDL3::SDL3)
