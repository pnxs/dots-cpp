
#find_file(DotsCodeGenerator_DCG dcg.py HINTS ${CMAKE_SOURCE_DIR}/import/dcg/bin)

set(DOTS_PYTHON_PATH "${CMAKE_SOURCE_DIR}/import/dcg/:${CMAKE_SOURCE_DIR}/model")
set(DOTS_CG_EXECUTABLE "${CMAKE_SOURCE_DIR}/import/dcg/bin/dcg.py")
set(DOTS_CG_CONFIG "config_cpp")
set(DOTS_CG PYTHONPATH=${DOTS_PYTHON_PATH} ${DOTS_CG_EXECUTABLE})

mark_as_advanced(DotsCodeGenerator_DCG)

#include(FindPackageHandleStandardArgs)
#find_package_handle_standard_args(DotsCodeGenerator_DCG
#        REQUIRED_VARS DotsCodeGenerator_DCG
#)

function(GET_GENERATED_DOTS_TYPES DF GENERATED_FILES)
    #message("Execute: ${DOTS_CG} -M ${DF}")
    execute_process(COMMAND "sh" "-c" "PYTHONPATH=${DOTS_PYTHON_PATH} ${DOTS_CG_EXECUTABLE} -C ${DOTS_CG_CONFIG} -M ${DF}"
            WORKING_DIRECTORY ..
            OUTPUT_VARIABLE generated_file_list
            RESULT_VARIABLE rv
            )
    string(REPLACE "\n" ";" out_files ${generated_file_list})
    #message("rv='${rv}'")
    set(${GENERATED_FILES} ${out_files} PARENT_SCOPE)
endfunction(GET_GENERATED_DOTS_TYPES)

function(GENERATE_DOTS_TYPES OUT_SOURCES OUT_HEADERS)
    foreach(DF ${ARGN})
        get_filename_component(Basename ${DF} NAME)
        GET_GENERATED_DOTS_TYPES(${DF} GENERATED_FILES)
        list(APPEND out_sources ${GENERATED_FILES})
        foreach(f ${GENERATED_FILES})
            if(f MATCHES "\\.dots\\.h$" )
                list(APPEND out_headers ${CMAKE_CURRENT_BINARY_DIR}/${f})
            endif()
        endforeach()
        #message("Output: ${GENERATED_FILES}")
        set(DOTS_TEMPLATES ${DOTS_TEMPLATE_PATH}/struct.dots.cpp.dotsT
                ${DOTS_TEMPLATE_PATH}/struct.dots.h.dotsT
                ${DOTS_TEMPLATE_PATH}/struct2.dots2.cpp.dotsT
                ${DOTS_TEMPLATE_PATH}/enum.dots.cpp.dotsT
                ${DOTS_TEMPLATE_PATH}/enum.dots.h.dotsT)

        #message("Custom command for ${DF}")
        #message("  OUTPUT ${GENERATED_FILES}")

        if(NOT IS_ABSOLUTE "${DF}")
            set(DF "${CMAKE_CURRENT_SOURCE_DIR}/${DF}")
        endif()

        add_custom_command(OUTPUT ${GENERATED_FILES}
                DEPENDS ${DOTS_TEMPLATES} ${DF}
                COMMAND ${DOTS_CG} -C ${DOTS_CG_CONFIG} -T ${DOTS_TEMPLATE_PATH} ${DF}
                COMMENT "Generate DOTS C++ classes from ${DF}"
                )

        set(${OUT_SOURCES} ${out_sources} PARENT_SCOPE)
        set(${OUT_HEADERS} ${out_headers} PARENT_SCOPE)

    endforeach()
    unset(DF)
    unset(DOTSTYPE_OUPUT)
endfunction(GENERATE_DOTS_TYPES)

install(CODE "
    execute_process(
        COMMAND /usr/bin/python setup.py install --prefix ${CMAKE_INSTALL_PREFIX}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/import/dcg
    )
")

install(FILES 
        ${DOTS_CG_EXECUTABLE}
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_READ OWNER_WRITE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)
