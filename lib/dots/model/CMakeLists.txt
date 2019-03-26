find_package(DotsCodeGenerator REQUIRED)
set(DOTSTYPES
        ${CMAKE_CURRENT_SOURCE_DIR}/dotstypes.dots
        ${CMAKE_CURRENT_SOURCE_DIR}/descriptors.dots
        ${CMAKE_CURRENT_SOURCE_DIR}/dotsmessages.dots)

GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

set(SOURCE_FILES ${SOURCES} ${NEW_HEADERS} ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

add_library(dots-model OBJECT ${SOURCE_FILES})
target_include_directories(dots-model
           PUBLIC
           $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
           $<INSTALL_INTERFACE:include>
        PRIVATE
              $<TARGET_PROPERTY:dots,INTERFACE_INCLUDE_DIRECTORIES>
)
set_property(TARGET dots-model PROPERTY POSITION_INDEPENDENT_CODE ON)

install(FILES ${HEADERS} DESTINATION include/dots-model)
if (NOT "${GEN_DOTS_HDR}" STREQUAL  "")
       #message("DOTSLIB: GEN_DOTS_HDR: ${GEN_DOTS_HDR}")
       install(FILES ${GEN_DOTS_HDR} DESTINATION include)

       #install(EXPORT ${LIBNAME}Config DESTINATION cmake)
       #export(TARGETS ${LIBNAME} FILE ${LIBNAME}Config.cmake)
endif()
