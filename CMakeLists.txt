cmake_minimum_required(VERSION 3.12)
project(dots-cpp
    VERSION 1.2
    DESCRIPTION "Distributed Objects in Time and Space - C++ implementation"
    LANGUAGES CXX
)

# options
add_compile_definitions(
    # force usage of uncaught_exceptions() in the date library because the corresponding C++17 handling
    # does not work properly for unknown reasons
    HAS_UNCAUGHT_EXCEPTIONS
)
add_definitions(-DENABLE_LOG_DEBUG -DENABLE_LOG_DATA -DBOOST_ERROR_CODE_HEADER_ONLY -DBOOST_SYSTEM_NO_DEPRECATED)
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(DOTS_TEMPLATE_PATH ${CMAKE_SOURCE_DIR}/model/template)
set(Boost_USE_STATIC_LIBS ON)
option(BUILD_TZ_LIB "" ON)
option(BUILD_UNIT_TESTS "Build the unit tests" ON)
if (UNIX)
    option(USE_SYSTEM_TZ_DB "" ON)
else()
    option(MANUAL_TZ_DB "" ON)
endif()
if (MSVC)
    # force shared CRT because GTest defaults to static CRT when it is itself linked statically
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    # use standard C++ exception model and assume no exceptions for external C functions
    # note that this needs to be set globally to include external targets because MSVC is disadvising
    # from mixing exceptions modes
    add_compile_options(/EHsc)
    # replace default warning level that CMake sets when using MSVC. the override code is based
    # on an example given by Kitware in [1]. note that this issue should already have been fixed since
    # CMake 3.15 (see [2]), but for some reason still occurs with the CMake 3.19.20122902-MSVC_2 version
    # used by Visual Studio 16.9.3
    #
    # References:
    #
    #  - [1] https://gitlab.kitware.com/cmake/community/wikis/FAQ#dynamic-replace
    #
    #  - [2] https://gitlab.kitware.com/cmake/cmake/-/issues/19084
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        string(REPLACE "/W3" "/W4" ${flag_var} "${${flag_var}}")
    endforeach()
endif()

# external projects
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
add_subdirectory(bin/examples/roundtrip)
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

# documentation
find_package(Doxygen)
if (Doxygen_FOUND)
    set(DOXYGEN_EXCLUDE_PATTERNS */external/* */tests/*)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ./README.md)
    doxygen_add_docs(dots-doc)
endif()
