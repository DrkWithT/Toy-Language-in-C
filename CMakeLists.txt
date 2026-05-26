cmake_minimum_required(VERSION 4.2)

project(toyscript VERSION 0.0.1 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")
set(PROJECT_SOURCES "${PROJECT_SOURCE_DIR}/src")
set(PROJECT_INCLUDES "${PROJECT_SOURCE_DIR}/src/includes")

add_library(tbasic_lang_impl)
target_include_directories(tbasic_lang_impl PUBLIC "${PROJECT_INCLUDES}")
target_sources(tbasic_lang_impl PRIVATE
    "${PROJECT_SOURCES}/mystr.c"
    "${PROJECT_SOURCES}/lex.c"
    "${PROJECT_SOURCES}/value.c"
    "${PROJECT_SOURCES}/objects.c"
    "${PROJECT_SOURCES}/obj_list.c"
    "${PROJECT_SOURCES}/obj_str.c"
    "${PROJECT_SOURCES}/obj_dict.c"
    "${PROJECT_SOURCES}/gc.c"
    "${PROJECT_SOURCES}/bytecode.c"
    "${PROJECT_SOURCES}/compiler.c"
    "${PROJECT_SOURCES}/vm.c"
    "${PROJECT_SOURCES}/driver.c"
)

add_executable(tbasic)
target_include_directories(tbasic PUBLIC "${PROJECT_INCLUDES}")
target_sources(tbasic PRIVATE
    "${PROJECT_SOURCES}/main.c"
)
target_link_libraries(tbasic PRIVATE tbasic_lang_impl)
