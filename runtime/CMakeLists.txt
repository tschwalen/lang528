# Minimum CMake version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project name and C language setting
project(MyRuntime LANGUAGES C)

include_directories(include)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Source files for your runtime
file(GLOB RUNTIME_SRCS src/*.c) 

# Create a static library for the runtime
add_library(runtime STATIC ${RUNTIME_SRCS})

# If main.c and tests exist, build an executable
set(MAIN_SRC main.c)

# Only create the executable if the main file exists
if(EXISTS "${CMAKE_SOURCE_DIR}/main.c")
    add_executable(runtime_exec ${MAIN_SRC})
    target_link_libraries(runtime_exec PRIVATE runtime)
endif()

# Optional clean target if needed
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} --build . --target clean
)
