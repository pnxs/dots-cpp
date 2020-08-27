get_filename_component(DOTS_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(CMakeFindDependencyMacro)

list(APPEND CMAKE_MODULE_PATH ${DOTS_CMAKE_DIR})
find_package(Boost 1.59 REQUIRED COMPONENTS filesystem iostreams program_options)
list(REMOVE_AT CMAKE_MODULE_PATH -1)

if(NOT TARGET DOTS)
    include("${DOTS_CMAKE_DIR}/DOTSTargets.cmake")
endif()

set(DOTS_LIBRARIES DOTS)