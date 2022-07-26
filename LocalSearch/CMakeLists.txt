project(item-assignment)

find_package(Boost 1.60 COMPONENTS system)

file(GLOB sources *.cc)
file(GLOB headers *.hh)
include(easylocal-ide)

#list(REMOVE_ITEM sources ${CMAKE_CURRENT_SOURCE_DIR}/SP_Main.cc)

set(SOURCE_FILES ${sources} ${headers})
add_executable(IA_solver ${SOURCE_FILES})
target_compile_options(IA_solver PUBLIC -Wall -Wpedantic)
target_link_libraries(IA_solver EasyLocal Boost::system IA_solver_input)
set_target_properties(IA_solver PROPERTIES CXX_STANDARD 17)
install(TARGETS IA_solver RUNTIME DESTINATION ${PROJECT_SOURCE_DIR}/../bin)