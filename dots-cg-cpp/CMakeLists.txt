# project
cmake_minimum_required(VERSION 3.12)
project(DOTS-CG-CPP LANGUAGES CXX)
set(TARGET_NAME dots-cg-cpp)
set(EXPORT_NAME DOTS-CG-CPP)

# code generation
set(DOTS-CG-CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/src CACHE INTERNAL "Internal helper variable containing the DOTS-CG-CPP directory")
include(src/DOTS-CG-CPP.cmake)

# install
install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/${EXPORT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
    COMPONENT dev
)
install(FILES
        src/DOTS-CG-CPP.cmake
    DESTINATION share/dots-cg-cpp
    COMPONENT dev
)
install(
    FILES
        src/config_cpp.py
        src/enum.dots.h.dotsT
        src/struct.dots.h.dotsT
    DESTINATION share/dots-cg-cpp
    COMPONENT dev
)
