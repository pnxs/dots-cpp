cmake_minimum_required(VERSION 3.8)
project(dots-test LANGUAGES CXX)
set(TARGET_NAME dots-test)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME} main.cpp)

# properties
target_sources(${TARGET_NAME}
	PRIVATE
)	
target_include_directories(${TARGET_NAME}
	PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(${TARGET_NAME}
	PRIVATE
		-O2 -flto -Wall -Wextra
)
target_compile_features(${TARGET_NAME}
	PRIVATE
		cxx_std_17
)
target_link_libraries(${TARGET_NAME}
	PRIVATE 
		DOTS::DOTS
)

# install
install (TARGETS ${TARGET_NAME} DESTINATION bin)