cmake_minimum_required(VERSION 3.0)
get_filename_component(app ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${app} main.cpp)
target_link_libraries(${app} nats_static)
