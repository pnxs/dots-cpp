
macro(DOTS_LIBRARY LIBNAME)
    set(LIB_NAME ${LIBNAME})

    file(GLOB SOURCES "*.cpp")
    file(GLOB HEADERS "*.h")
    file(GLOB DOTSTYPES "*.dots")

    GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

    set(SOURCE_FILES ${SOURCES} ${NEW_HEADERS} ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

    add_library(${LIB_NAME} ${SOURCE_FILES})
    target_include_directories(${LIB_NAME}
            PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
                $<INSTALL_INTERFACE:include>
            )
    #target_include_directories(${LIBNAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

    install(TARGETS ${LIBNAME} EXPORT ${LIBNAME}Config
            DESTINATION lib
    )
    install(FILES ${HEADERS} DESTINATION include/${LIBNAME})
    if (NOT "${GEN_DOTS_HDR}" STREQUAL  "")
    #message("DOTSLIB: GEN_DOTS_HDR: ${GEN_DOTS_HDR}")
    install(FILES ${GEN_DOTS_HDR} DESTINATION include)

    install(EXPORT ${LIBNAME}Config DESTINATION cmake)
    export(TARGETS ${LIBNAME} FILE ${LIBNAME}Config.cmake)

    endif()

endmacro(DOTS_LIBRARY)

macro(DOTS_BINARY BINNAME)
    set(BIN_NAME ${BINNAME})

    file(GLOB_RECURSE SOURCES "*.cpp")
    file(GLOB_RECURSE HEADERS "*.h")
    file(GLOB_RECURSE DOTSTYPES "*.dots")

    GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

    set(SOURCE_FILES ${SOURCES} ${NEW_HEADERS} ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

    set (extra_macro_args ${ARGN})
    list(LENGTH extra_macro_args num_extra_args)
    if (${num_extra_args} GREATER 0)
        list(GET extra_macro_args 0 optional_arg)
        #message ("Got an optional arg: ${optional_arg}")
    endif ()

    add_executable(${BIN_NAME} ${SOURCE_FILES})
    target_link_libraries(${BIN_NAME} ${optional_arg} Boost::system pthread)
    target_include_directories(${BIN_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    install(TARGETS ${BINNAME} DESTINATION bin)

endmacro(DOTS_BINARY)
