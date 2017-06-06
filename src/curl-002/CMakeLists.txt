cmake_minimum_required(VERSION 3.0)
get_filename_component(app ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_executable(${app} main.cpp)

find_package(CURL REQUIRED)
target_include_directories(${app} PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(${app} ${CURL_LIBRARIES})
