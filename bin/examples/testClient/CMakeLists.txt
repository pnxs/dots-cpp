cmake_minimum_required(VERSION 3.12)
project(testClient LANGUAGES CXX)
set(TARGET_NAME testClient)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME} src/main.cpp)

# properties
target_sources(${TARGET_NAME}
    PRIVATE
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror>>
        $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>>:/W4>
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
install (TARGETS ${TARGET_NAME} DESTINATION bin COMPONENT testclient)
