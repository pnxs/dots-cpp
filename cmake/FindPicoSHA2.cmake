find_package(PkgConfig)
pkg_check_modules(PC_PicoSHA2 QUIET PicoSHA2)

set(PC_PicoSHA2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../external)
find_path(PicoSHA2_INCLUDE_DIR
    NAMES picosha2.h
    PATHS ${PC_PicoSHA2_INCLUDE_DIRS}
    PATH_SUFFIXES PicoSHA2
)

set(PicoSHA2_VERSION ${PC_PicoSHA2_VERSION})

mark_as_advanced(PicoSHA2_FOUND PicoSHA2_INCLUDE_DIR PicoSHA2_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PicoSHA2
    REQUIRED_VARS PicoSHA2_INCLUDE_DIR
    VERSION_VAR PicoSHA2_VERSION
)

if(PicoSHA2_FOUND)
    get_filename_component(PicoSHA2_INCLUDE_DIRS ${PicoSHA2_INCLUDE_DIR} DIRECTORY)
endif()

if(PicoSHA2_FOUND AND NOT TARGET PicoSHA2::PicoSHA2)
    add_library(PicoSHA2::PicoSHA2 INTERFACE IMPORTED)
    set_target_properties(PicoSHA2::PicoSHA2 PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PicoSHA2_INCLUDE_DIRS}"
    )
endif()
