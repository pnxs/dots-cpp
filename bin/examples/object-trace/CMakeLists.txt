cmake_minimum_required(VERSION 3.12)
project(object-trace LANGUAGES CXX)
set(TARGET_NAME ${PROJECT_NAME})

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is not part of build tree)

# target
add_executable(${TARGET_NAME})

# properties
target_sources(${TARGET_NAME}
    PRIVATE
        src/main.cpp
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror>>
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
