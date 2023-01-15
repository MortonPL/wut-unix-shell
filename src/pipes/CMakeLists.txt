# sources
set(SRC_LIB_FILES
    # main
    pipes.c
)

add_library(pipes ${SRC_LIB_FILES})
target_include_directories(pipes PUBLIC "${CMAKE_SOURCE_DIR}/src/pipes")
target_link_libraries(pipes PRIVATE uuid)
