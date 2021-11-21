cmake_minimum_required(VERSION 3.12)
project(smart-home LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is not part of build tree)
#find_package(GTest REQUIRED) (uncomment when dependency is not part of build tree)

# target
add_executable(${TARGET_NAME})

# properties
target_dots_model(${TARGET_NAME}
    src/model.dots
)
target_sources(${TARGET_NAME}
    PRIVATE
        src/main.cpp
        src/Basement.cpp
        src/LivingRoom.cpp
        src/Stairwell.cpp
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)
target_compile_features(${TARGET_NAME}
    PRIVATE
        cxx_std_17
)
target_link_libraries(${TARGET_NAME}
    PRIVATE
        DOTS::DOTS
)

# install
install (TARGETS ${TARGET_NAME} DESTINATION bin COMPONENT ${TARGET_NAME})

# tests
set(TEST_TARGET_NAME ${TARGET_NAME}-tests)

# target [tests]
add_executable(${TEST_TARGET_NAME})

# properties [tests]
target_dots_model(${TEST_TARGET_NAME}
    src/model.dots
)
target_sources(${TEST_TARGET_NAME}
    PRIVATE
        tests/BasementTest.cpp
        tests/LivingRoomTest.cpp
        tests/StairwellTest.cpp
        src/Basement.cpp
        src/LivingRoom.cpp
        src/Stairwell.cpp
)
target_include_directories(${TEST_TARGET_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TEST_TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_compile_definitions(${TEST_TARGET_NAME}
    PRIVATE
        # suppress warning for usage of unsafe C runtime functions (e.g. getenv)
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)
target_compile_features(${TEST_TARGET_NAME}
    PRIVATE
        cxx_std_17
)
target_link_libraries(${TEST_TARGET_NAME}
    PRIVATE
        DOTS::DOTS
        gmock
        gtest
        gtest_main
)
