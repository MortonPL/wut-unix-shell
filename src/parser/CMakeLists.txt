execute_process(
    COMMAND "flex" "scanner.lex"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE RES)
if(NOT (RES EQUAL 0))
    message(FATAL_ERROR "flex failed with message: ${RES}")
endif()

# sources
set(SRC_PARSER_FILES
    lex.yy.c
    parser.c
    parser.tab.c
)

# add the library version
add_library(parser ${SRC_PARSER_FILES})
target_include_directories(parser PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(parser PRIVATE fl)
