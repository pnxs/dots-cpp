function(bundle_static_library tgt_name)
  list(APPEND static_libs_full_names $<TARGET_FILE:${tgt_name}>)

  function(_recursively_collect_dependencies input_target)
    set(_input_link_libraries LINK_LIBRARIES)
    get_target_property(_input_type ${input_target} TYPE)
    if (${_input_type} STREQUAL "INTERFACE_LIBRARY")
      set(_input_link_libraries INTERFACE_LINK_LIBRARIES)
    endif()
    get_target_property(public_dependencies ${input_target} ${_input_link_libraries})
    foreach(dependency IN LISTS public_dependencies)
      string(REGEX REPLACE [[\$<BUILD_INTERFACE:([^>]+)>]] [[\1]] dependency ${dependency})
      if(TARGET ${dependency})
        get_target_property(alias ${dependency} ALIASED_TARGET)
        if (TARGET ${alias})
          set(dependency ${alias})
        endif()
        get_target_property(_type ${dependency} TYPE)

        if (${_type} STREQUAL "STATIC_LIBRARY")
          list(APPEND static_libs_full_names $<TARGET_FILE:${dependency}>)
        endif()

        get_property(library_already_added
          GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency})
        if (NOT library_already_added)
          set_property(GLOBAL PROPERTY _${tgt_name}_static_bundle_${dependency} ON)
          _recursively_collect_dependencies(${dependency})
        endif()
      endif()
    endforeach()
    set(static_libs_full_names ${static_libs_full_names} PARENT_SCOPE)
  endfunction()

  _recursively_collect_dependencies(${tgt_name})

  list(REMOVE_DUPLICATES static_libs_full_names)

  set(bundled_tgt_name ${tgt_name}_bundle)

  set(bundled_tgt_full_name
    ${CMAKE_BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${bundled_tgt_name}${CMAKE_STATIC_LIBRARY_SUFFIX})

  set(tgt_full_name $<TARGET_FILE:${tgt_name}>)

  if (CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(ar_tool ${CMAKE_AR})
    if (CMAKE_INTERPROCEDURAL_OPTIMIZATION)
      set(ar_tool ${CMAKE_CXX_COMPILER_AR})
    endif()

    add_custom_command(TARGET ${tgt_name} POST_BUILD
      COMMAND printf "CREATE ${bundled_tgt_full_name}\\nADDLIB $<JOIN:${static_libs_full_names},\\nADDLIB >\\nSAVE\\nEND" > ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      COMMAND ${ar_tool} -M < ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      COMMAND ${CMAKE_COMMAND} -E copy ${bundled_tgt_full_name} ${tgt_full_name}
      COMMAND ${CMAKE_COMMAND} -E remove ${bundled_tgt_full_name}
      BYPRODUCTS ${CMAKE_BINARY_DIR}/${bundled_tgt_name}.ar
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM
      )
  elseif(MSVC)
    find_program(lib_tool lib)

    add_custom_command(TARGET ${tgt_name} POST_BUILD
      COMMAND ${lib_tool} /NOLOGO /OUT:${bundled_tgt_full_name} ${static_libs_full_names}
      COMMAND ${CMAKE_COMMAND} -E copy ${bundled_tgt_full_name} ${tgt_full_name}
      COMMAND ${CMAKE_COMMAND} -E remove ${bundled_tgt_full_name}
      COMMENT "Bundling ${bundled_tgt_name}"
      VERBATIM)
  else()
    message(FATAL_ERROR "Unknown bundle scenario!")
  endif()

endfunction()
