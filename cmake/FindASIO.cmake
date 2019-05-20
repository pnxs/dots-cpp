find_package(PkgConfig)
pkg_check_modules(PC_ASIO QUIET ASIO)

set(PC_ASIO_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/import/asio/asio/include)
find_path(ASIO_INCLUDE_DIR
    NAMES asio.hpp
    PATHS ${PC_ASIO_INCLUDE_DIRS}
    PATH_SUFFIXES asio
)

set(ASIO_VERSION ${PC_ASIO_VERSION})

mark_as_advanced(ASIO_FOUND ASIO_INCLUDE_DIR ASIO_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ASIO
    REQUIRED_VARS ASIO_INCLUDE_DIR
    VERSION_VAR ASIO_VERSION
)

if(ASIO_FOUND AND NOT TARGET ASIO::ASIO)
    add_library(ASIO::ASIO INTERFACE IMPORTED)
    set_target_properties(ASIO::ASIO PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ASIO_INCLUDE_DIR}"
    )
endif()