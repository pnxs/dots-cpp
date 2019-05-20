find_path(Sqlite3_INCLUDE_DIR sqlite3.h)
find_library(Sqlite3_LIBRARY sqlite3)
mark_as_advanced(Sqlite3_INCLUDE_DIR Sqlite3_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Sqlite3
        REQUIRED_VARS Sqlite3_INCLUDE_DIR Sqlite3_LIBRARY
        )

if (Sqlite3_FOUND AND NOT TARGET Sqlite3::Sqlite3)
    add_library(Sqlite3::Sqlite3 UNKNOWN IMPORTED)
    set_target_properties(Sqlite3::Sqlite3 PROPERTIES
            IMPORTED_LOCATION "${Sqlite3_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Sqlite3_INCLUDE_DIR}"
            )
endif()