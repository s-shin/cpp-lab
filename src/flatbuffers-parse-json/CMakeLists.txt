cmake_minimum_required(VERSION 3.0)
find_program(FLATC flatc)
add_custom_command(
  OUTPUT cpplab_generated.h
  COMMAND ${FLATC} --cpp ${CMAKE_CURRENT_SOURCE_DIR}/cpplab.fbs
)
get_filename_component(app ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${app} cpplab_generated.h main.cpp)
target_include_directories(${app} PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(${app} flatbuffers)
