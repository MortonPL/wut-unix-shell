# directories
add_subdirectory(log.c/src)

# sources
set(SRC_LIB_FILES
    # main
    mmem.c
    logger.c
    log.c/src/log.c
)

add_library(lib ${SRC_LIB_FILES})
target_include_directories(lib PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
