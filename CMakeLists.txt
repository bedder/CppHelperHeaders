cmake_minimum_required(VERSION 3.0)

project("CppHelperExamples")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

include_directories("${PROJECT_SOURCE_DIR}/include")

add_executable(Optional ${PROJECT_SOURCE_DIR}/source/Optional.cpp)
