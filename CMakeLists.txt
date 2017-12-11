cmake_minimum_required(VERSION 3.0)

project(cpp-lab CXX C)

list(APPEND CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/deps)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

add_compile_options(-std=c++11 -Wall -Wnon-virtual-dtor -Woverloaded-virtual -fvisibility=hidden)

link_directories(deps/lib)
include_directories(deps/include)

add_compile_options(-DASIO_STANDALONE)

file(GLOB app_dirs src/*)
foreach(dir ${app_dirs})
  add_subdirectory(${dir})
endforeach(dir)
