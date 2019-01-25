
add_executable(dotsd main.cpp
        ConnectionManager.cpp ConnectionManager.h
        Connection.cpp Connection.h
        Server.cpp Server.h
        Group.cpp Group.h
        GroupManager.cpp GroupManager.h
        AuthManager.cpp AuthManager.h
        ServerInfo.h
)

target_link_libraries(dotsd dots Boost::system pthread)

install(TARGETS dotsd DESTINATION sbin)

add_subdirectory(tests)
