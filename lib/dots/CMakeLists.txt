# project
cmake_minimum_required(VERSION 3.8)
project(libDOTS VERSION 0.1.0 LANGUAGES CXX)
set(TARGET_NAME dots)
set(EXPORT_NAME DOTS)

# options
option(BUILD_DOTS_SHARED "Build DOTS as a shared instead of a static library" OFF)

# dependencies
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)
find_package(ASIO REQUIRED MODULE)
find_package(Boost 1.59 REQUIRED COMPONENTS filesystem iostreams program_options)
find_package(Rapidjson REQUIRED MODULE)
#find_package(cborcpp REQUIRED) (uncomment when dependency is no longer part of build tree)
find_package(Websocketpp REQUIRED MODULE)

# target
if (BUILD_DOTS_SHARED)
	add_library(${TARGET_NAME} SHARED)
else()
	add_library(${TARGET_NAME} STATIC)
endif()
add_library(${EXPORT_NAME}::${EXPORT_NAME} ALIAS ${TARGET_NAME})

# properties
GENERATE_DOTS_TYPES(MODEL_SOURCES MODEL_HEADERS
	model/dotstypes.dots
    model/descriptors.dots
    model/dotsmessages.dots
	model/dotstesttypes.dots
)
target_sources(${TARGET_NAME}
	PRIVATE
		dots.cpp
		dots_base.cpp
		Uuid.cpp

		common/Chrono.cpp
		common/logging.cpp
		common/Stream.cpp
		common/string_convert.cpp
		common/seconds.cpp

		functional/signal.cpp

		io/Transmitter.cpp
		io/Transceiver.cpp
		io/ServerConnection.cpp
		io/Subscription.cpp
		io/Chained.cpp

		io/Io.cpp
		io/Container.cpp
		io/ContainerPool.cpp
		io/Dispatcher.cpp
		io/Event.cpp
		io/TD_Traversal.cpp
		io/Application.cpp
		io/DynamicTypeReceiver.cpp
		io/ResourceUsage.cpp
		io/DistributedTypeId.cpp
		io/Filter.cpp
		
		io/serialization/JsonSerialization.cpp
		io/serialization/AsciiSerialization.cpp
		io/serialization/CborNativeSerialization.cpp

		io/services/Channel.cpp
		io/services/FdHandlerService.cpp
		io/services/Listener.cpp
		io/services/TcpListener.cpp
		io/services/TcpChannel.cpp
		io/services/Timer.cpp
		io/services/TimerService.cpp
		io/services/Transmission.cpp
		io/services/VirtualChannel.cpp
		io/services/WebSocketChannel.cpp
		io/services/WebSocketListener.cpp

		io/recording/FileStore.cpp
		io/recording/Load.cpp
		io/recording/MmapedFile.cpp
		io/recording/Playback.cpp
		io/recording/Record.cpp
		io/recording/Store.cpp
		io/recording/ContinuousRecord.cpp
		io/recording/JsonStore.cpp
		io/recording/DynamicLoader.cpp
		io/recording/Player.cpp
		io/recording/FileReader.cpp
		type/AnyStruct.cpp
		type/ChronoDescriptor.cpp
		type/Descriptor.cpp
		type/Enum.cpp
		type/EnumDescriptor.cpp
		type/PropertyIterator.cpp
		type/PropertyPairIterator.cpp
		type/EnumDescriptor.cpp
		type/Registry.cpp
		type/StandardTypes.cpp
		type/StructDescriptor.cpp
		type/StructProperty.cpp
		type/UuidDescriptor.cpp
		type/Vector.cpp
		type/VectorDescriptor.cpp
		type/Struct.cpp
		
		type/NewDescriptor.cpp
		type/NewPropertyDescriptor.cpp
		type/NewProxyPropertyIterator.cpp
		type/NewProxyPropertyPairIterator.cpp
		type/NewVectorDescriptor.cpp

		${MODEL_SOURCES}
)
target_include_directories(${TARGET_NAME}
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(${TARGET_NAME}
	PUBLIC
		cxx_std_17
)
target_compile_options(${TARGET_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -flto -Wall -Wextra -Wpedantic -Werror>>		
)
target_compile_definitions(${TARGET_NAME}
	PUBLIC
		$<BUILD_INTERFACE:ASIO_STANDALONE>
		$<BUILD_INTERFACE:ASIO_DISABLE_EPOLL>
)
target_link_libraries(${TARGET_NAME}
	PUBLIC
		$<BUILD_INTERFACE:ASIO::ASIO>
		$<BUILD_INTERFACE:Websocketpp::websocketpp>
		Boost::boost
		Boost::program_options
		Boost::iostreams
		Boost::filesystem
		pthread
	PRIVATE
		cborcpp
		RapidJSON::RapidJSON		
)
set_target_properties(${TARGET_NAME} PROPERTIES
    SOVERSION 0
)

# install
include(GNUInstallDirs)
set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME ${EXPORT_NAME})
install(TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(EXPORT ${TARGET_NAME}-targets
	NAMESPACE ${EXPORT_NAME}::
	FILE
		${EXPORT_NAME}Targets.cmake
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
)
install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/${EXPORT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
)
install(DIRECTORY
	.
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TARGET_NAME}
	FILES_MATCHING PATTERN "*.h"
)

# tests
if (BUILD_UNIT_TESTS)
	add_subdirectory(tests)
endif()