cmake_minimum_required(VERSION 3.12)
project(dotsd LANGUAGES CXX)
set(TARGET_NAME dotsd)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME})

# properties
target_sources(${TARGET_NAME}
    PRIVATE
        src/main.cpp
        src/DotsDaemon.cpp
)
target_include_directories(${TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_compile_definitions(${TARGET_NAME}
    PRIVATE
        DOTS_NO_GLOBAL_TRANSCEIVER
)
target_compile_features(${TARGET_NAME}
    PRIVATE
        cxx_std_20
)
target_link_libraries(${TARGET_NAME}
    PRIVATE
        DOTS::DOTS
)

# install
install (TARGETS ${TARGET_NAME} DESTINATION sbin COMPONENT main)
