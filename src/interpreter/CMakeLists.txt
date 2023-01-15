# sources
set(SRC_LIB_FILES
    # main
    cli.c
    interpreter.c
)

add_library(interpreter ${SRC_LIB_FILES})
target_include_directories(interpreter PUBLIC "${CMAKE_SOURCE_DIR}/src/interpreter")
