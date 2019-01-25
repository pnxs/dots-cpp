
find_program( GCOV gcov )
find_program( GENHTML genhtml )
find_program( LCOV lcov )

set(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
set(GCC_COVERAGE_LINK_FLAGS    "--coverage")

function(JOIN OUTPUT GLUE)
    set(_TMP_RESULT "")
    set(_GLUE "")
    foreach(arg ${ARGN})
        message(STATUS "arg: ${arg}")
        set(_TMP_RESULT "${_TMP_RESULT}${_GLUE}${arg}")
        set(_GLUE "${GLUE}")
    endforeach()
    set(${OUTPUT} "${_TMP_RESULT}" PARENT_SCOPE)
endfunction()

function(EXPAND OUTPUT BEG END)
    set(_TMP_RESULT "")
    foreach(arg ${ARGN})
        LIST(APPEND _TMP_RESULT "${BEG}${arg}${END}")
    endforeach()
    set(${OUTPUT} ${_TMP_RESULT} PARENT_SCOPE)
endfunction()

function(add_coverage_target _targetname _testrunner _output)
    if (NOT LCOV)
        message(FATAL_ERROR "lcov not found!")
    endif()

    #expand(DIRLIST "-d ../" "" ${ARGN})
    expand(DIRLIST "-d " "" ${ARGN})
    separate_arguments(DIRLIST)

    add_custom_target(${_targetname}
        ${LCOV} -d . -z
        COMMAND ${LCOV} -c -i ${DIRLIST} -d . -o ${_output}.base.info
        COMMAND ${_testrunner} ${ARGV3}

        #COMMAND ${LCOV} ${DIRLIST} -d . --capture --no-external -o ${_output}.test.info
        COMMAND ${LCOV} ${DIRLIST} -d . --capture -o ${_output}.test.info
        COMMAND ${LCOV} -a ${_output}.base.info -a ${_output}.test.info -o ${_output}.info
        COMMAND ${LCOV} --remove ${_output}.info 'tests/*' '/usr/*' --output-file ${_output}.info.cleaned
        COMMAND ${GENHTML} -o ${_output} ${_output}.info.cleaned
        COMMAND ${CMAKE_COMMAND} -E remove ${_output}.info ${_output}.info.cleaned

        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endfunction() # add_coverage_target
