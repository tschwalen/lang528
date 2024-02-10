cmake_minimum_required(VERSION 3.10)

execute_process(COMMAND scripts/codegen.py)

project(output)

include_directories(include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCES "src/*.cpp")

add_executable(output ${SOURCES})