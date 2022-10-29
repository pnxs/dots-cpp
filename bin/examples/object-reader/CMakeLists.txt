cmake_minimum_required(VERSION 3.12)
project(object-reader LANGUAGES CXX)
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
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror -Wno-missing-field-initializers>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
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
install (TARGETS ${TARGET_NAME} DESTINATION bin COMPONENT ${TARGET_NAME})
