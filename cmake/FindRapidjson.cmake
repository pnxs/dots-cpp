
add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/import/rapidjson/include>)

#set_property(TARGET Rapidjson::rapidjson PROPERTY
#        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/import/rapidjson/include)


install(TARGETS rapidjson EXPORT RapidJsonConfig)
install(EXPORT RapidJsonConfig DESTINATION cmake)
export(TARGETS rapidjson FILE RapidJsonConfig.cmake)