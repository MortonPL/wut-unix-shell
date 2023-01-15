#CMake basics
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0048 NEW)

# set the project name
project(shell VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the standards/options
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

# set binary names
set(LIB_NAME ${PROJECT_NAME}lib)
set(EXE_NAME ${PROJECT_NAME})
set(TEST_NAME ${PROJECT_NAME}test)

# partially borrowed from Tau's CMakeLists
if(NOT MSVC)
    # standard
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-std=gnu11)
    else()
        add_compile_options(-std=c11)
    endif()

    # Optimizations
    if(CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        add_compile_options(-O2)
    endif()
    
    # extra flags
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
    add_compile_options(-Wpedantic)
    add_compile_options(-Wconversion)
    
    # enable Werror if possible
    if (CMAKE_BUILD_TYPE STREQUAL "RELEASE")
        include(CheckCCompilerFlag)
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.10.0")
            check_c_compiler_flag("-Werror" COMPILER_SUPPORT_WERROR)
        else()
            CHECK_C_COMPILER_FLAG("-Werror" COMPILER_SUPPORT_WERROR)
        endif()
        if(COMPILER_SUPPORT_WERROR)
            add_compile_options(-Werror)   
        endif()
    endif()
endif()

add_subdirectory(src)
add_subdirectory(test)
