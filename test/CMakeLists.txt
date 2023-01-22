enable_testing()

# sources
set(TEST_FILES
    main.c
    parser.c
    interpreter.c
)

add_executable(${TEST_NAME} ${TEST_FILES})
target_include_directories(${TEST_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tau")
target_link_libraries(${TEST_NAME} PRIVATE ${LIB_NAME})
