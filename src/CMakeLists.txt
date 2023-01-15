# directories
add_subdirectory(lib)
add_subdirectory(parser)

# sources
set(SRC_FILES
    # main
    main.c
)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_include_directories(${LIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(${LIB_NAME} PUBLIC parser lib)

# add the executable
add_executable(${EXE_NAME} ${SRC_FILES})
target_include_directories(${EXE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})
