# sources
set(SRC_LIB_FILES
    # main
    log.c
)

add_library(log.c ${SRC_LIB_FILES})
target_include_directories(log.c PUBLIC "${CMAKE_SOURCE_DIR}/src/log.c/src")
target_link_libraries(log.c PRIVATE)
