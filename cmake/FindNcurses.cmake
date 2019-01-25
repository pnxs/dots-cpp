find_path(Ncurses_INCLUDE_DIR curses.h)
find_library(Ncurses_LIBRARY ncurses)
mark_as_advanced(Ncurses_INCLUDE_DIR Ncurses_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Ncurses
        REQUIRED_VARS Ncurses_INCLUDE_DIR Ncurses_LIBRARY
        )

if (Ncurses_FOUND AND NOT TARGET Ncurses::Ncurses)
    add_library(Ncurses::Ncurses UNKNOWN IMPORTED)
    set_target_properties(Ncurses::Ncurses PROPERTIES
            IMPORTED_LOCATION "${Ncurses_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${Ncurses_INCLUDE_DIR}"
            )
endif()