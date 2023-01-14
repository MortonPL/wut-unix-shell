# sources
set(SRC_LIB_FILES
    # main
    mmem.c
    strpls.c
)

add_library(lib ${SRC_LIB_FILES})
target_include_directories(lib PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
