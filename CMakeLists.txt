cmake_minimum_required(VERSION 3.6)
project(DOTS LANGUAGES CXX)

set(CXX_STANDARD_REQUIRED true)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_definitions(-DENABLE_LOG_DEBUG -DENABLE_LOG_DATA)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(DOTS_TEMPLATE_PATH ${CMAKE_SOURCE_DIR}/model/template)

option(WITH_QT5   "Build with Qt5 support" OFF)
option(BUILD_XDOTS "Build the xdots utility" ON)
option(BUILD_XDOTS_STATIC "Link the xdots-utility static" OFF)
option(BUILD_NDOTS "Build the ndots utility" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)

add_definitions(-DDOTS_GET_WARN_ON_NOT_VALID)
#add_definitions(-DDOTS_GET_FAIL_ON_NOT_VALID)

include(cmake/DOTS.cmake)

set(Boost_USE_STATIC_LIBS ON)

find_package(Rapidjson REQUIRED)
find_package(Boost 1.59 REQUIRED COMPONENTS filesystem iostreams program_options system)
find_package(DotsCodeGenerator REQUIRED)
find_package(Sqlite3)

add_subdirectory(lib/dots)
add_subdirectory(lib/dots_model_extra)

if (Sqlite3_FOUND)
    add_subdirectory(lib/sqlitecpp)
    add_subdirectory(bin/dots-persistd)
endif()

add_subdirectory(bin/dotsd)
add_subdirectory(bin/dots-web-connector)
add_subdirectory(bin/utils/dotstrace)
add_subdirectory(bin/examples/dots-test)
add_subdirectory(bin/utils/dots_record)
add_subdirectory(bin/utils/dotscli)
add_subdirectory(bin/examples/testClient)

if (BUILD_XDOTS)
    add_subdirectory(bin/utils/xdots)
endif()

if (WITH_QT5)
    add_subdirectory(bin/utils/qdots2)
    add_subdirectory(bin/utils/dots_qplay)
endif()

if (BUILD_NDOTS)
    add_subdirectory(bin/utils/ndots)
endif()

add_subdirectory(model)

add_subdirectory(import/googletest)
add_subdirectory(import/cbor-cpp)

install(FILES cmake/FindDOTS.cmake DESTINATION cmake)
