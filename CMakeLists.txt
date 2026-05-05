cmake_minimum_required(VERSION 4.2)

project(toyscript VERSION 0.0.1 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")
set(PROJECT_SOURCES "${PROJECT_SOURCE_DIR}/src")
set(PROJECT_INCLUDES "${PROJECT_SOURCE_DIR}/src/includes")

add_library(toyscript_impl)
target_include_directories(toyscript_impl PUBLIC "${PROJECT_INCLUDES}")
target_sources(toyscript_impl PRIVATE
    "${PROJECT_SOURCES}/mystr.c"
    "${PROJECT_SOURCES}/lex.c"
)

add_executable(toyscript)
target_include_directories(toyscript PUBLIC "${PROJECT_INCLUDES}")
target_sources(toyscript PRIVATE
    "${PROJECT_SOURCES}/main.c"
)
target_link_libraries(toyscript PRIVATE toyscript_impl)
