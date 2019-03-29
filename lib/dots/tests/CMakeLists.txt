cmake_minimum_required(VERSION 3.8)
project(dots-unittests LANGUAGES CXX)
set(TARGET_NAME dots-unittests)

# CMake options
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

# dependencies
#find_package(DOTS REQUIRED) (uncomment when dependency is no longer part of build tree)

# target
add_executable(${TARGET_NAME} TestAnyContainer.cpp)

# properties
target_sources(${TARGET_NAME}
	PRIVATE
		TestAnyContainer.cpp
        TestContainer.cpp
        TestAsciiSerialization.cpp
        TestCborSerialization.cpp
        TestChronoDescriptor.cpp
        TestDistributedTypeId.cpp
        TestEnumDescriptor.cpp
        TestFileStore.cpp
        TestJsonSerialization.cpp
        TestPropertyIterator.cpp
		TestPropertyProxy.cpp
		TestPropertyProxyVoid.cpp
		TestPropertySet.cpp
        TestRecord.cpp
        TestRegistry.cpp
        TestStruct.cpp
		TestTProperty.cpp
		TestTStruct.cpp
        TestVectorDescriptor.cpp
        TestJsonStore.cpp
        TestFileReader.cpp
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
		gmock
		gtest
		gtest_main
)