# FindDOTS
#
# Locate and configure the DOTS library
#
# Parameters:
#
# DOTS_ROOT
#
#
# Defines following:
#
# ``DOTS_FOUND``
# Set to true, if the DOTS library was found
# ``DOTS_INCLUDE_DIRS``
# Include directories of DOTS
# ``DOTS_LIBRARIES``
#

######################

function(GET_GENERATED_DOTS_TYPES DF GENERATED_FILES)
    #message("Execute: ${DOTS_CG_EXECUTABLE} -M ${DF}")
    execute_process(COMMAND "sh" "-c" "PYTHONPATH=${DOTS_PYTHON_PATH} ${DOTS_CG_EXECUTABLE} -C ${DCG_CONFIG} -M ${DF}"
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
                COMMAND "sh" "-c" "PYTHONPATH=${DOTS_PYTHON_PATH} ${DOTS_CG_EXECUTABLE} -C ${DCG_CONFIG} -T ${DOTS_TEMPLATE_PATH} ${DF}"
                COMMENT "Generate DOTS C++ classes from ${DF}"
        )

        set(${OUT_SOURCES} ${out_sources} PARENT_SCOPE)
        set(${OUT_HEADERS} ${out_headers} PARENT_SCOPE)

    endforeach()
    unset(DF)
    unset(DOTSTYPE_OUPUT)
endfunction(GENERATE_DOTS_TYPES)

macro(DOTS_LIBRARY LIBNAME)
    set(LIB_NAME ${LIBNAME})

    file(GLOB SOURCES "*.cpp")
    file(GLOB HEADERS "*.h")
    file(GLOB DOTSTYPES "*.dots")

    GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

    #STAGE_HEADERS(NEW_HEADERS ${HEADERS} ${GEN_DOTS_HDR})

    set(SOURCE_FILES ${SOURCES} ${NEW_HEADERS} ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

    add_library(${LIB_NAME} ${SOURCE_FILES})
    target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    install(TARGETS ${LIBNAME} DESTINATION lib)
    install(FILES ${HEADERS} DESTINATION include/${LIBNAME})
    if (NOT "${GEN_DOTS_HDR}" STREQUAL  "")
    #message("DOTSLIB: GEN_DOTS_HDR: ${GEN_DOTS_HDR}")
    install(FILES ${GEN_DOTS_HDR} DESTINATION include)
    endif()

endmacro(DOTS_LIBRARY)

macro(DOTS_BINARY BINNAME)
    set(BIN_NAME ${BINNAME})

    file(GLOB SOURCES "*.cpp")
    file(GLOB HEADERS "*.h")
    file(GLOB DOTSTYPES "*.dots")

    GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

    #STAGE_HEADERS(NEW_HEADERS ${HEADERS} ${GEN_DOTS_HDR})

    set(SOURCE_FILES ${SOURCES} ${NEW_HEADERS} ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

    set (extra_macro_args ${ARGN})
    list(LENGTH extra_macro_args num_extra_args)
    if (${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 optional_arg)
        #message ("Got an optional arg: ${optional_arg}")
    endif ()

    add_executable(${BIN_NAME} ${SOURCE_FILES})
    target_link_libraries(${BIN_NAME} ${optional_arg} ${DOTS_LIBRARIES} Boost::system pthread)
    target_include_directories(${BIN_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    install(TARGETS ${BINNAME} DESTINATION bin)

endmacro(DOTS_BINARY)

######################

find_program(DOTS_CG_EXECUTABLE
        NAMES dcg.py
        DOC "The DOTS type compiler"
        )

find_path(DOTS_TEMPLATE_PATH
        NAMES struct.dots.h.dotsT
        PATH_SUFFIXES share/dots)

find_path(DCG_CONFIG_PATH
        NAMES config_cpp.py
        PATH_SUFFIXES share/dots)

set(_DOTS_SEARCH_INCLUDES
        HINTS
        ENV DOTS_INCLUDE_DIR
        ENV DOTS_ROOT
        )

set(_DOTS_SEARCH_LIBS
        HINTS
        ENV DOTS_LIBRARY_DIR
        ENV DOTS_ROOT
        )

find_path(DOTS_INCLUDE_DIR
        dots/io/Transceiver.h
        ${_DOTS_SEARCH_INCLUDES}
        PATH_SUFFIXES include
        )

find_path(DOTS_PYTHON_PATH
        NAMES dots/parser.py
        PATH_SUFFIXES lib/python2.7/site-packages
        )

message("DOTS_PYTHON_PATH: ${DOTS_PYTHON_PATH}")
message("DOTS_INCLUDE_DIR: ${DOTS_INCLUDE_DIR}")

find_library(DOTS_CORE_LIB NAMES dots ${_DOTS_SEARCH_LIBS} PATH_SUFFIXES lib .)

find_library(DOTS_LIBRARIES NAMES dots ${_DOTS_SEARCH_LIBS}
        PATH_SUFFIXES
        lib
        .
        )

set(DOTS_LIBRARIES ${DOTS_CORE_LIB})

message("DOTS_LIBRARIES: ${DOTS_LIBRARIES}")

set(DOTS_PYTHON_PATH "${DOTS_PYTHON_PATH}:${DCG_CONFIG_PATH}")
set(DCG_CONFIG "config_cpp")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DOTS DEFAULT_MSG DOTS_LIBRARIES DOTS_INCLUDE_DIR DOTS_TEMPLATE_PATH DCG_CONFIG_PATH DOTS_CG_EXECUTABLE)

mark_as_advanced(DOTS_INCLUDE_DIR DOTS_LIBRARIES)

set(DOTS_INCLUDE_DIRS ${DOTS_INCLUDE_DIR})

find_package(Boost 1.59 REQUIRED COMPONENTS filesystem iostreams program_options system)

find_package(dots REQUIRED)
