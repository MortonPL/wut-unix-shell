# directories
add_subdirectory(lib)
add_subdirectory(parser)
add_subdirectory(pipes)
add_subdirectory(log.c/src)

# sources
set(SRC_FILES
    # main
    main.c
)

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_include_directories(${LIB_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(${LIB_NAME} PUBLIC parser pipes lib log.c)

# add the executable
add_executable(${EXE_NAME} ${SRC_FILES})
target_include_directories(${EXE_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(${EXE_NAME} PRIVATE ${LIB_NAME})

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    target_compile_definitions(${LIB_NAME} PUBLIC DEBUG)
endif()
