#CMake basics
cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)

# set the project name
project(UXP VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the standards/options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set binary names
set(EXE_NAME ${PROJECT_NAME})
set(LIB_NAME ${PROJECT_NAME}_lib)
set(TEST_NAME ${PROJECT_NAME}_test)

add_subdirectory(src)
add_subdirectory(test)