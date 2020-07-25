# project
cmake_minimum_required(VERSION 3.8)
project(libDOTS VERSION 0.1.0 LANGUAGES CXX)
set(TARGET_NAME dots)
set(EXPORT_NAME DOTS)

# options
option(BUILD_DOTS_SHARED "Build DOTS as a shared instead of a static library" OFF)

# dependencies
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)
find_package(Boost 1.72 REQUIRED COMPONENTS filesystem iostreams program_options)
find_package(Rapidjson REQUIRED MODULE)
find_package(PicoSHA2 REQUIRED)
#find_package(cborcpp REQUIRED) (uncomment when dependency is no longer part of build tree)

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
        Application.cpp

        tools/logging.cpp
        tools/signal.cpp
        tools/IpNetwork.cpp

        io/Transceiver.cpp
        io/GuestTransceiver.cpp
        io/HostTransceiver.cpp
        io/Connection.cpp
        io/Subscription.cpp

        io/Io.cpp
        io/Container.cpp
        io/ContainerPool.cpp
        io/Dispatcher.cpp
        io/Event.cpp

        io/Channel.cpp
        io/Listener.cpp
        io/Medium.cpp

        io/serialization/JsonSerialization.cpp
        io/serialization/AsciiSerialization.cpp
        io/serialization/CborNativeSerialization.cpp

        io/channels/TcpListener.cpp
        io/channels/TcpChannel.cpp
        io/channels/UdsChannel.cpp
        io/channels/UdsListener.cpp
        io/channels/VirtualChannel.cpp
        io/channels/WebSocketChannel.cpp
        io/channels/WebSocketListener.cpp

        io/services/FdHandlerService.cpp
        io/Timer.cpp
        io/services/TimerService.cpp
        io/Transmission.cpp

        io/Registry.cpp
        io/DescriptorConverter.cpp

        io/auth/Nonce.cpp
        io/auth/Digest.cpp
        io/auth/AuthManager.cpp

        type/AnyStruct.cpp
        type/Chrono.cpp
        type/Descriptor.cpp
        type/DynamicStruct.cpp
        type/EnumDescriptor.cpp
        type/PropertyDescriptor.cpp
        type/PropertyPath.cpp
        type/PropertyPairIterator.cpp
        type/PropertyIterator.cpp
        type/Struct.cpp
        type/StructDescriptor.cpp
        type/Uuid.cpp
        type/VectorDescriptor.cpp

        ${MODEL_SOURCES}
)
target_include_directories(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/dots-cpp/lib>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(${TARGET_NAME}
    PUBLIC
        cxx_std_17
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -flto -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations>>
)
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:BOOST_ASIO_DISABLE_EPOLL>
)
target_link_libraries(${TARGET_NAME}
    PUBLIC
        Boost::boost
        Boost::program_options
        Boost::iostreams
        Boost::filesystem
        pthread
    PRIVATE
        cborcpp
        RapidJSON::RapidJSON
        date::date
        date::tz
        PicoSHA2::PicoSHA2
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