cmake_minimum_required(VERSION 3.8)
project(dotsd LANGUAGES CXX)
set(TARGET_NAME dotsd)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME} main.cpp)

# properties
target_sources(${TARGET_NAME}
	PRIVATE
		AuthManager.cpp
        Connection.cpp
		ConnectionManager.cpp
		Group.cpp
        GroupManager.cpp
        Server.cpp
)	
target_include_directories(${TARGET_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
	PRIVATE
		-g -O2 -Wall -Wextra
)
target_compile_features(${TARGET_NAME}
	PRIVATE
		cxx_std_17
)
target_link_libraries(${TARGET_NAME}
	PRIVATE 
		DOTS::DOTS
		pthread
)

# install
install (TARGETS ${TARGET_NAME} DESTINATION sbin)

# tests
if (BUILD_UNIT_TESTS)
	add_subdirectory(tests)
endif()