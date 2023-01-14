add_library(fl STATIC IMPORTED)
set_target_properties(fl PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/flex/src/.libs/libfl.a"
  #INTERFACE_INCLUDE_DIRECTORIES
)

execute_process(
    COMMAND "flex/src/flex" "main.lex"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE RES)
if(NOT (RES EQUAL 0))
    message(FATAL_ERROR "flex failed with message: ${RES}")
endif()

# sources
set(SRC_LEXER_FILES
    lex.yy.c
    lexer.c
)

# add the library version
add_library(lexer ${SRC_LEXER_FILES})
target_include_directories(lexer PUBLIC "${CMAKE_SOURCE_DIR}/src/lib")
target_link_libraries(lexer PRIVATE fl)
