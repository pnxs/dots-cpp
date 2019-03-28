# helper variables
set(DOTS-CG-CPP_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "Internal helper variable containing the DOTS-CG-CPP directory")
set(DOTS-CG_CONFIG "config_cpp" CACHE INTERNAL "Internal helper variable containing the DOTS-CG config file name for C++")
set(DOTS-CG_TEMPLATE_DIR ${DOTS-CG-CPP_DIR} CACHE INTERNAL "Internal helper variable containing the DOTS-CG template directory")
set(DOTS-CG_TEMPLATE_LIST 
	${DOTS-CG-CPP_DIR}/struct.dots.h.dotsT;
	${DOTS-CG-CPP_DIR}/enum.dots.h.dotsT;
	${DOTS-CG-CPP_DIR}/enum.dots.cpp.dotsT
	CACHE INTERNAL "Internal helper variable containing the C++ code generation templates"
)

# code generation
function(GET_GENERATED_DOTS_TYPES GENERATED_TYPES MODEL)
    execute_process(COMMAND "sh" "-c" "${DOTS-CG} -C ${DOTS-CG_CONFIG} -T ${DOTS-CG_TEMPLATE_DIR} -M ${MODEL}"
        OUTPUT_VARIABLE generated_types_list
        RESULT_VARIABLE rv
    )
    string(REPLACE "\n" ";" out_types ${generated_types_list})
    set(${GENERATED_TYPES} ${out_types} PARENT_SCOPE)
endfunction(GET_GENERATED_DOTS_TYPES)

function(GENERATE_DOTS_TYPES GENERATED_SOURCES GENERATED_HEADERS)
    foreach(MODEL ${ARGN})
		# gather generated source and header files
		get_filename_component(Basename ${MODEL} NAME)
        GET_GENERATED_DOTS_TYPES(GENERATED_TYPES ${MODEL})
        list(APPEND generated_sources ${GENERATED_TYPES})
        foreach(TYPE ${GENERATED_TYPES})
            if(TYPE MATCHES "\\.dots\\.h$" )
                list(APPEND generated_headers ${CMAKE_CURRENT_BINARY_DIR}/${TYPE})
            endif()
        endforeach()

		# ensure that model path is absolute
        if(NOT IS_ABSOLUTE "${MODEL}")
            set(MODEL "${CMAKE_CURRENT_SOURCE_DIR}/${MODEL}")
        endif()        

		# create generation target for all types in model
        add_custom_command(OUTPUT ${GENERATED_TYPES}
			COMMAND "sh" "-c" "${DOTS-CG} -C ${DOTS-CG_CONFIG} -T ${DOTS-CG_TEMPLATE_DIR} ${MODEL}"
            DEPENDS ${DOTS-CG_TEMPLATE_LIST} ${MODEL}            
            COMMENT "Generate DOTS C++ classes from ${MODEL}"
        )

		set(${GENERATED_SOURCES} ${generated_sources} PARENT_SCOPE)
        set(${GENERATED_HEADERS} ${generated_headers} PARENT_SCOPE)
    endforeach()
endfunction(GENERATE_DOTS_TYPES)