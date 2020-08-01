cmake_minimum_required(VERSION 3.8)
project(dots LANGUAGES CXX)

# options
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

# sub-projects
add_subdirectory(dots-cpp)
add_subdirectory(dots-cpp-legacy)