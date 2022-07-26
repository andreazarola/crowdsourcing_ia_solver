project(item-assignment-input)

find_package(Boost 1.60 COMPONENTS system)

file(GLOB sources *.cc)
file(GLOB headers *.hh)
#include(easylocal-ide)

#list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/SP_Main.cc)

set(SOURCE_FILES ${sources} ${headers})
add_library(IA_solver_input ${SOURCE_FILES})
target_compile_options(IA_solver_input PUBLIC -Wall -Wpedantic)
target_compile_features(IA_solver_input PUBLIC cxx_std_17)
target_link_libraries(IA_solver_input Boost::system)
set_target_properties(IA_solver_input PROPERTIES  CXX_STANDARD 17)
