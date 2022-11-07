# directories

# sources
set(SRC_FILES
    # main
    main.cpp
)

# add the executable
add_executable(${EXE_NAME} ${SRC_FILES})
target_link_libraries(${EXE_NAME})

# add the library version
add_library(${LIB_NAME} ${SRC_FILES})
target_link_libraries(${LIB_NAME})
