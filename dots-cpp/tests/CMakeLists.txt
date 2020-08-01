cmake_minimum_required(VERSION 3.8)
project(dots-unittests LANGUAGES CXX)
set(TARGET_NAME dots-unittests)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME} src/TestContainer.cpp)

# properties
target_sources(${TARGET_NAME}
    PRIVATE
        src/TestDigest.cpp
        src/TestAsciiSerialization.cpp
        src/TestCborSerialization.cpp
        src/TestChrono.cpp
        src/TestDispatcher.cpp
        src/TestJsonSerialization.cpp
        src/TestEnumDescriptor.cpp
        src/TestStaticDescriptor.cpp
        src/TestStaticStruct.cpp
        src/TestDynamicStruct.cpp
        src/TestPropertySet.cpp
        src/TestProperty.cpp
        src/TestPropertyIterator.cpp
        src/TestProxyProperty.cpp
        src/TestIpNetwork.cpp
        src/TestLegacyAuthManager.cpp
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -flto -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations>>
)
target_compile_features(${TARGET_NAME}
    PRIVATE
        cxx_std_17
)
target_link_libraries(${TARGET_NAME}
    PRIVATE
        DOTS::DOTS
        gmock
        gtest
        gtest_main
        date::date
        date::tz
)