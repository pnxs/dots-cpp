cmake_minimum_required(VERSION 3.8)
project(dots LANGUAGES CXX)

# options
add_definitions(-DENABLE_LOG_DEBUG -DENABLE_LOG_DATA -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)
set(DOTS_TEMPLATE_PATH ${CMAKE_SOURCE_DIR}/model/template)
set(Boost_USE_STATIC_LIBS ON)
option(BUILD_TZ_LIB "" ON)
option(USE_SYSTEM_TZ_DB "" ON)

option(WITH_QT5   "Build with Qt5 support" OFF)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)

# optional dependencies
find_package(Sqlite3)

# external projects
add_subdirectory(import/googletest EXCLUDE_FROM_ALL)
add_subdirectory(import/cbor-cpp EXCLUDE_FROM_ALL)
add_subdirectory(import/date EXCLUDE_FROM_ALL)

# sub-projects
add_subdirectory(dots-cpp/dots-cg-cpp)
add_subdirectory(dots-cpp/dots)
add_subdirectory(lib/dots_legacy)
add_subdirectory(bin/dotsd)
add_subdirectory(bin/dots-web-connector)
add_subdirectory(bin/utils/dotsctl)
add_subdirectory(bin/examples/testClient)

if (Sqlite3_FOUND)
    add_subdirectory(lib/sqlitecpp)
    add_subdirectory(bin/dots-persistd)
endif()

if (WITH_QT5)
    add_subdirectory(bin/utils/qdots2)
    add_subdirectory(bin/utils/dots_qplay)
endif()