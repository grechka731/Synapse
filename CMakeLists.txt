cmake_minimum_required(VERSION 3.16)

project(EngineProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(engine)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE engine_submodule)