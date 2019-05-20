find_package(PkgConfig)
pkg_check_modules(PC_Websocketpp QUIET Websocketpp)

set(PC_Websocketpp_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/import/websocketpp/websocketpp)
find_path(Websocketpp_INCLUDE_DIR
    NO_DEFAULT_PATHS
    PATHS ${PC_Websocketpp_INCLUDE_DIRS}
    NAMES version.hpp
)
set(Websocketpp_INCLUDE_DIR ${Websocketpp_INCLUDE_DIR}/../)
set(Websocketpp_VERSION ${PC_Websocketpp_VERSION})

mark_as_advanced(Websocketpp_FOUND Websocketpp_INCLUDE_DIR Websocketpp_VERSION)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Websocketpp
    REQUIRED_VARS Websocketpp_INCLUDE_DIR
    VERSION_VAR Websocketpp_VERSION
)

if(Websocketpp_FOUND AND NOT TARGET Websocketpp::websocketpp)
    add_library(Websocketpp::websocketpp INTERFACE IMPORTED)
    set_target_properties(Websocketpp::websocketpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Websocketpp_INCLUDE_DIR}"
    )
endif()
