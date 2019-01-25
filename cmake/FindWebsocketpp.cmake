
add_library(websocketpp INTERFACE)
target_include_directories(websocketpp
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/import/websocketpp>)

#set_property(TARGET Rapidjson::rapidjson PROPERTY
#        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/import/rapidjson/include)


#install(TARGETS websocketpp EXPORT websocketppConfig)
#install(EXPORT websocketppConfig DESTINATION cmake)
#export(TARGETS websocketpp FILE websocketppConfig.cmake)

add_library(Websocketpp::websocketpp ALIAS websocketpp)
