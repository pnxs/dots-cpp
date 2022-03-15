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
    src/serialization/serialization.dots
    src/type/TestStaticStruct.dots
)
target_sources(${TARGET_NAME}
    PRIVATE
        src/serialization/TestCborSerializer.cpp
        src/serialization/TestExperimentalCborSerializer.cpp
        src/serialization/TestJsonSerializer.cpp
        src/serialization/TestRapidJsonSerializer.cpp
        src/serialization/TestStringSerializer.cpp

        src/type/TestRegistry.cpp

        src/tools/TestUri.cpp

        src/io/auth/TestDigest.cpp
        src/serialization/TestAsciiSerialization.cpp
        src/type/TestChrono.cpp
        src/TestDispatcher.cpp
        src/type/TestEnumDescriptor.cpp
        src/type/TestStaticDescriptor.cpp
        src/type/TestStaticStruct.cpp
        src/type/TestDynamicStruct.cpp
        src/type/TestPropertySet.cpp
        src/type/TestProperty.cpp
        src/type/TestPropertyIterator.cpp
        src/type/TestProxyProperty.cpp
        src/type/TestDescriptor.cpp
        src/tools/TestIpNetwork.cpp
        src/io/auth/TestLegacyAuthManager.cpp
        src/TestHostTransceiver.cpp
        src/TestGuestTransceiver.cpp
        src/TestConnection.cpp
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-gnu-zero-variadic-macro-arguments>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_compile_definitions(${TARGET_NAME}
    PRIVATE
        # suppress warning for usage of unsafe C runtime functions (e.g. getenv)
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
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
        date::date-tz
        RapidJSON::RapidJSON
)

# target [dots-unittests-experimental]
add_executable(${TARGET_NAME_EXPERIMENTAL} src/type/TestStaticStructExperimental.cpp)

# properties [dots-unittests-experimental]
#target_sources(${TARGET_NAME_EXPERIMENTAL}
#    PRIVATE
#)
target_dots_model(${TARGET_NAME_EXPERIMENTAL}
    src/type/TestStaticStruct.dots
)
target_include_directories(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-gnu-zero-variadic-macro-arguments>>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>>:/W4>
)
target_compile_definitions(${TARGET_NAME_EXPERIMENTAL}
    PRIVATE
        # suppress warning for usage of unsafe C runtime functions (e.g. getenv)
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
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
        date::date-tz
)
