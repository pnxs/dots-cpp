# project
cmake_minimum_required(VERSION 3.8)
project(DOTS-CG-CPP LANGUAGES CXX)
set(TARGET_NAME dots-cg-cpp)
set(EXPORT_NAME DOTS-CG-CPP)

# code generation
set(DOTS-CG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../external/dcg)
set(DOTS-CG-CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(DOTS-CG "PYTHONPATH=${DOTS-CG-CPP_DIR}:${DOTS-CG_DIR} ${DOTS-CG_DIR}/bin/dcg.py" CACHE INTERNAL "Internal variable containing the DOTS-CG command")
include(src/DOTS-CG-CPP.cmake)

# install
install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/${EXPORT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
)
install(FILES
        src/DOTS-CG-CPP.cmake
    DESTINATION share/dots-cg-cpp
)
install(
    FILES
        ${DOTS-CG_DIR}/bin/dcg.py
    DESTINATION share/dots-cg-cpp
    PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
install(
    CODE
        "execute_process(
            COMMAND /usr/bin/python setup.py install --prefix ${CMAKE_INSTALL_PREFIX}/share/dots-cg-cpp
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../external/dcg
        )"
)
install(
    FILES
        src/config_cpp.py
        src/enum.dots.cpp.dotsT
        src/enum.dots.h.dotsT
        src/struct.dots.h.dotsT
    DESTINATION share/dots-cg-cpp
)