cmake_minimum_required(VERSION 3.5)
project(item-assignment LANGUAGES CXX)

list(INSERT CMAKE_MODULE_PATH 0 ${PROJECT_SOURCE_DIR}/easylocal-3/cmake)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/LocalSearch)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
endif()

include_directories(${PROJECT_SOURCE_DIR}/InputData)

add_subdirectory(easylocal-3)
add_subdirectory(InputData)
add_subdirectory(LocalSearch)
