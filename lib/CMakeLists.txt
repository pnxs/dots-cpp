# project
cmake_minimum_required(VERSION 3.12)
project(libDOTS VERSION 0.1.0 LANGUAGES CXX)
set(TARGET_NAME dots)
set(EXPORT_NAME DOTS)

include(LibraryUtils)

# options
option(BUILD_DOTS_SHARED "Build DOTS as a shared instead of a static library" OFF)

# dependencies
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)
find_package(Boost 1.71 REQUIRED COMPONENTS filesystem iostreams program_options)
find_package(RapidJSON REQUIRED MODULE)
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
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/descriptors.dots
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/dotsmessages.dots
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/dotstypes.dots
    src/model/authentication.dots
    src/model/dotstesttypes.dots
)
target_sources(${TARGET_NAME}
    PRIVATE
        src/dots.cpp
        src/Application.cpp

        src/tools/logging.cpp
        src/tools/signal.cpp
        src/tools/IpNetwork.cpp

        src/io/Transceiver.cpp
        src/io/GuestTransceiver.cpp
        src/io/HostTransceiver.cpp
        src/io/Connection.cpp
        src/io/Subscription.cpp

        src/io/Io.cpp
        src/io/Container.cpp
        src/io/ContainerPool.cpp
        src/io/Dispatcher.cpp
        src/io/Event.cpp

        src/io/Channel.cpp
        src/io/Listener.cpp
        src/io/Medium.cpp

        src/io/serialization/JsonSerialization.cpp
        src/io/serialization/AsciiSerialization.cpp
        src/io/serialization/CborNativeSerialization.cpp

        src/io/channels/TcpListener.cpp
        src/io/channels/TcpChannel.cpp
        src/io/channels/UdsChannel.cpp
        src/io/channels/UdsListener.cpp
        src/io/channels/VirtualChannel.cpp
        src/io/channels/WebSocketChannel.cpp
        src/io/channels/WebSocketListener.cpp

        src/io/services/FdHandlerService.cpp
        src/io/Timer.cpp
        src/io/services/TimerService.cpp
        src/io/Transmission.cpp

        src/io/Registry.cpp
        src/io/DescriptorConverter.cpp

        src/io/auth/Nonce.cpp
        src/io/auth/Digest.cpp
        src/io/auth/AuthManager.cpp
        src/io/auth/LegacyAuthManager.cpp

        src/type/AnyStruct.cpp
        src/type/Chrono.cpp
        src/type/Descriptor.cpp
        src/type/DynamicEnum.cpp
        src/type/DynamicStruct.cpp
        src/type/EnumDescriptor.cpp
        src/type/PropertyDescriptor.cpp
        src/type/PropertyPath.cpp
        src/type/PropertyPairIterator.cpp
        src/type/PropertyIterator.cpp
        src/type/StaticDescriptor.cpp
        src/type/Struct.cpp
        src/type/StructDescriptor.cpp
        src/type/Uuid.cpp
        src/type/VectorDescriptor.cpp

        ${MODEL_SOURCES}
)
target_include_directories(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(${TARGET_NAME}
    PUBLIC
        cxx_std_17
)
target_compile_options(${TARGET_NAME}
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-O2 -Wall -Wextra -Wpedantic -Werror -Wno-deprecated-declarations>>
)
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        $<BUILD_INTERFACE:BOOST_ASIO_DISABLE_EPOLL>
    PRIVATE
        DOTS_NO_GLOBAL_TRANSCEIVER
)
target_link_libraries(${TARGET_NAME}
    PUBLIC
        Boost::boost
        Boost::program_options
        Boost::iostreams
        Boost::filesystem
        # link pthread which is required for C++ threads on Linux and OSX
        $<$<OR:$<PLATFORM_ID:Linux>,$<PLATFORM_ID:Darwin>>:pthread>
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

bundle_static_library(${TARGET_NAME})

# Strip libs
get_property(ILL
    TARGET ${TARGET_NAME}
    PROPERTY INTERFACE_LINK_LIBRARIES)
list(FILTER ILL EXCLUDE REGEX ".*cborcpp.*")
list(FILTER ILL EXCLUDE REGEX ".*date::.*")
list(FILTER ILL EXCLUDE REGEX ".*RapidJSON::.*")
list(FILTER ILL EXCLUDE REGEX ".*PicoSHA2::.*")
list(LENGTH ILL mylength)
set_property(
    TARGET ${TARGET_NAME}
    PROPERTY INTERFACE_LINK_LIBRARIES ${ILL})

# install
include(GNUInstallDirs)
set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME ${EXPORT_NAME})
install(TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT dev
)
install(EXPORT ${TARGET_NAME}-targets
    NAMESPACE ${EXPORT_NAME}::
    FILE
        ${EXPORT_NAME}Targets.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
    COMPONENT dev
)
install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/${EXPORT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${EXPORT_NAME}
    COMPONENT dev
)
install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev
)
install(FILES
    ${MODEL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev
)
