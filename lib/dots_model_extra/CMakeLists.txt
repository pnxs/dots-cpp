set(DOTSTYPES ${CMAKE_CURRENT_SOURCE_DIR}/dotsmodel_extra.dots)

GENERATE_DOTS_TYPES(GEN_DOTS_SRC GEN_DOTS_HDR ${DOTSTYPES})

set(SOURCE_FILES ${GEN_DOTS_SRC} ${GEN_DOTS_HDR})

add_library(dots_model_extra ${SOURCE_FILES})
target_link_libraries(dots_model_extra dots)
target_include_directories(dots-model
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        $<TARGET_PROPERTY:dots,INTERFACE_INCLUDE_DIRECTORIES>
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:dots,INTERFACE_INCLUDE_DIRECTORIES>>
)