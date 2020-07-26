cmake_minimum_required(VERSION 3.8)
project(dots LANGUAGES CXX)

# options
option(BUILD_TZ_LIB "" ON)
option(USE_SYSTEM_TZ_DB "" ON)

# external projects
add_subdirectory(import/googletest EXCLUDE_FROM_ALL)
add_subdirectory(import/cbor-cpp EXCLUDE_FROM_ALL)
add_subdirectory(import/date EXCLUDE_FROM_ALL)

# sub-projects
add_subdirectory(dots-cpp)
add_subdirectory(dots-legacy)