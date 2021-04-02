cmake_minimum_required(VERSION 3.12)
project(dots-unittests LANGUAGES CXX)
set(TARGET_NAME dots-unittests)
set(TARGET_NAME_EXPERIMENTAL dots-unittests-experimental)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)
find_package(RapidJSON REQUIRED MODULE)

# target [dots-unittests]
add_executable(${TARGET_NAME} src/TestContainer.cpp)

# properties [dots-unittests]
target_dots_model(${TARGET_NAME}
    src/io/serialization/serialization.dots
)
target_sources(${TARGET_NAME}
    PRIVATE
        src/io/serialization/TestCborSerializer.cpp
        src/io/serialization/TestExperimentalCborSerializer.cpp
        src/io/serialization/TestJsonSerializer.cpp
        src/io/serialization/TestRapidJsonSerializer.cpp
        src/io/serialization/TestSerializerBase.cpp
        src/io/serialization/TestStringSerializer.cpp

        src/tools/TestUri.cpp

        src/TestDigest.cpp
        src/TestAsciiSerialization.cpp
        src/TestChrono.cpp
        src/TestDispatcher.cpp
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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations>>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>>:/W4>
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
        RapidJSON::RapidJSON
)

# target [dots-unittests-experimental]
add_executable(${TARGET_NAME_EXPERIMENTAL} src/TestStaticStructExperimental.cpp)

# properties [dots-unittests-experimental]
#target_sources(${TARGET_NAME_EXPERIMENTAL}
#    PRIVATE
#)
target_include_directories(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations -Wno-missing-field-initializers>>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>>:/W4>
)
target_compile_features(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        cxx_std_20
)
target_link_libraries(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        DOTS::DOTS
        gmock
        gtest
        gtest_main
        date::date
        date::tz
)