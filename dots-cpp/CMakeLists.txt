cmake_minimum_required(VERSION 3.8)
project(dots-cpp LANGUAGES CXX)

# options
add_definitions(-DENABLE_LOG_DEBUG -DENABLE_LOG_DATA -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(DOTS_TEMPLATE_PATH ${CMAKE_SOURCE_DIR}/model/template)
set(Boost_USE_STATIC_LIBS ON)
option(BUILD_TZ_LIB "" ON)
option(USE_SYSTEM_TZ_DB "" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)

# external projects
add_subdirectory(external/cbor-cpp EXCLUDE_FROM_ALL)
add_subdirectory(external/date EXCLUDE_FROM_ALL)
add_subdirectory(external/googletest EXCLUDE_FROM_ALL)

# sub-projects
add_subdirectory(dots-cg-cpp)
add_subdirectory(lib)
add_subdirectory(bin/dotsd)
add_subdirectory(bin/examples/testClient)