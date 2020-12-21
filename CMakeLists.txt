cmake_minimum_required(VERSION 3.12)
project(dots-cpp
    VERSION 1.1
    DESCRIPTION "Distributed Objects in Time and Space - C++ implementation"
    LANGUAGES CXX
)

# options
add_definitions(-DENABLE_LOG_DEBUG -DENABLE_LOG_DATA -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(DOTS_TEMPLATE_PATH ${CMAKE_SOURCE_DIR}/model/template)
set(Boost_USE_STATIC_LIBS ON)
option(BUILD_TZ_LIB "" ON)
option(USE_SYSTEM_TZ_DB "" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)
if (MSVC)
    # force shared CRT because GTest defaults to static CRT when it is itself linked statically
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # use standard C++ exception model and assume no exceptions for external C functions
    # note that this needs to be set globally to include external targets because MSVC is disadvising 
    # from mixing exceptions modes
    add_compile_options(/EHsc)
endif()

# external projects
add_subdirectory(external/cbor-cpp EXCLUDE_FROM_ALL)
add_subdirectory(external/date EXCLUDE_FROM_ALL)
add_subdirectory(external/googletest EXCLUDE_FROM_ALL)

# allow external packages to be found when cross-compiling
if (${CMAKE_CROSSCOMPILING})
    list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/)
endif()

# sub-projects
add_subdirectory(dots-cg-cpp)
add_subdirectory(lib)
add_subdirectory(bin/dotsd)
add_subdirectory(bin/examples/testClient)
if (BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Thomas Schätzlein")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.31), libstdc++6, libboost-program-options1.71.0")
set(CPACK_DEBIAN_MAIN_PACKAGE_NAME dots-cpp)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_MAIN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_DEV_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_TESTCLIENT_FILE_NAME DEB-DEFAULT)
set(CPACK_DEB_COMPONENT_INSTALL ON)

set(CPACK_STRIP_FILES TRUE)
set(CPACK_GENERATOR "DEB")

include(CPack)