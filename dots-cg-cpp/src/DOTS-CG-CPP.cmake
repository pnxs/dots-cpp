# helper variables
if (WIN32)
    find_package(Python3 3.7 REQUIRED COMPONENTS Interpreter)
    execute_process(COMMAND ${Python3_EXECUTABLE} -m site --user-site
        OUTPUT_VARIABLE Python3_SITEUSER
        RESULT_VARIABLE rv
    )
    if (${rv} GREATER 2)
        message(FATAL_ERROR "Could not determine Python3 user site-package location: ${rv}")
    endif()
    string(REPLACE "\n" "" Python3_SITEUSER ${Python3_SITEUSER})
    find_program(DOTS-CG NAMES dcg.py PATHS ${Python3_SITEARCH} ${Python3_SITEUSER} PATH_SUFFIXES bin)
    if(NOT ${DOTS-CG} STREQUAL DOTS-CG-NOTFOUND)
        set(DOTS-CG "${Python3_EXECUTABLE} ${DOTS-CG}")
    endif()
else()
    find_program(DOTS-CG NAMES dcg.py)
endif (WIN32)

if(${DOTS-CG} STREQUAL DOTS-CG-NOTFOUND)
    message(FATAL_ERROR "Could not find DOTS code generator")
endif()
if(NOT DEFINED DOTS-CG-CPP_DIR)
    message(FATAL_ERROR "DOTS-CG-CPP_DIR variable is not set")
endif()

set(DOTS-CG_CONFIG "config_cpp" CACHE INTERNAL "Internal helper variable containing the DOTS-CG config file name for C++")
set(DOTS-CG_TEMPLATE_DIR ${DOTS-CG-CPP_DIR} CACHE INTERNAL "Internal helper variable containing the DOTS-CG template directory")
set(DOTS-CG_TEMPLATE_LIST
    ${DOTS-CG-CPP_DIR}/struct.dots.h.dotsT;
    ${DOTS-CG-CPP_DIR}/enum.dots.h.dotsT;
    CACHE INTERNAL "Internal helper variable containing the C++ code generation templates"
)
set(DOTS-CG-CPP-GENERATE_CMD 
    ${DOTS-CG} --config=${DOTS-CG-CPP_DIR}/${DOTS-CG_CONFIG}.py --templatePath=${DOTS-CG_TEMPLATE_DIR}
    CACHE INTERNAL "Internal helper variable containing the DOTS-CG generate command"
)

function(target_dots_model TARGET_NAME)
    foreach(MODEL_FILE ${ARGN})
        # ensure that model file path is absolute
        if(NOT IS_ABSOLUTE "${MODEL_FILE}")
            set(MODEL_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL_FILE}")
        endif()

        # determine generated model header paths
        execute_process(COMMAND ${DOTS-CG-CPP-GENERATE_CMD} --list-generated ${MODEL_FILE} 
            OUTPUT_VARIABLE MODEL_TYPES 
            RESULT_VARIABLE rv
        )
        if (NOT ${rv} MATCHES "0")
            message(FATAL_ERROR "Could not generate type list: ${rv}")
        endif()
        string(REPLACE "\n" ";" MODEL_TYPES ${MODEL_TYPES})
        foreach(MODEL_TYPE ${MODEL_TYPES})
            list(APPEND MODEL_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${MODEL_TYPE})
        endforeach()

        # create header generation command for all types in model
        add_custom_command(OUTPUT ${MODEL_TYPES}
            COMMAND ${DOTS-CG-CPP-GENERATE_CMD} ${MODEL_FILE}
            DEPENDS ${DOTS-CG_TEMPLATE_LIST} ${MODEL_FILE}
            COMMENT "Generating DOTS C++ types for model file: ${MODEL_FILE}"
        )
    endforeach()
    add_custom_target(${TARGET_NAME}-generate ALL
        DEPENDS ${MODEL_HEADERS}
        SOURCES ${MODEL_HEADERS}
    )
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}-generate)
    set(${TARGET_NAME}-MODEL_HEADERS ${MODEL_HEADERS} PARENT_SCOPE)
endfunction(target_dots_model)
