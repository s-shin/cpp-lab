cmake_minimum_required(VERSION 3.0)

project(cpp-lab CXX C)

set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/deps)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_compile_options(-Wall -Wnon-virtual-dtor -Woverloaded-virtual -fvisibility=hidden)
# MEMO
# * `-Wall`
#   * 含まれないwarning: http://qiita.com/EqualL2/items/0cfb5b1cbfd70e2af662
#   * `-Wnon-virtual-dtor`
#   * `-Woverloaded-virtual`
# * `-fvisibility`
#   * http://gihyo.jp/lifestyle/serial/01/ganshiki-soushi/0034

find_package(gflags REQUIRED)
include_directories (${gflags_INCLUDE_DIR})

add_subdirectory(src/ex1)
add_subdirectory(src/ex2)
