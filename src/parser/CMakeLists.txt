SET(GEN_PARSER OFF CACHE BOOL "Regenerate parser files")
if (GEN_PARSER)
    find_program(FLEX "flex")
    if(FLEX)
        execute_process(
            COMMAND "${FLEX}" "-d" "lexer.l"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE FLEXRES)
        if(NOT (FLEXRES EQUAL 0))
            message(FATAL_ERROR "flex failed with message: ${FLEXRES}")
        endif()
    endif()

    find_program(BISON "bison")
    if(BISON)
        execute_process(
            COMMAND "${BISON}" "-t" "parser.y"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            RESULT_VARIABLE BISONRES)
        if(NOT (BISONRES EQUAL 0))
            message(FATAL_ERROR "bison failed with message: ${BISONRES}")
        endif()
    endif()
endif()

# sources
set(SRC_PARSER_FILES
    interface.c
    lexer.c
    parser.c
    structures.c
)

# add the library version
add_library(parser "${SRC_PARSER_FILES}")
target_include_directories(parser PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(parser PRIVATE fl)
