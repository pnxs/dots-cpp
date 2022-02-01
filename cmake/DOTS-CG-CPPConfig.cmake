if(NOT TARGET DOTS-CG-CPP)
    set(DOTS-CG-CPP_DIR ${CMAKE_CURRENT_LIST_DIR}/../../../share/dots-cg-cpp CACHE INTERNAL "Internal helper variable containing the DOTS-CG-CPP directory")
    include("${DOTS-CG-CPP_DIR}/DOTS-CG-CPP.cmake")
endif()
