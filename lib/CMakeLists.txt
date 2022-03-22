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
find_package(Boost 1.71 REQUIRED COMPONENTS program_options)
find_package(RapidJSON REQUIRED MODULE)
find_package(PicoSHA2 REQUIRED)

# target
if (BUILD_DOTS_SHARED)
    add_library(${TARGET_NAME} SHARED)
else()
    add_library(${TARGET_NAME} STATIC)
endif()

add_library(${EXPORT_NAME}::${EXPORT_NAME} ALIAS ${TARGET_NAME})

# properties
target_dots_model(${TARGET_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/connection.dots
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/event.dots
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/user.dots
    ${CMAKE_CURRENT_LIST_DIR}/../external/dots/model/descriptors.dots
    src/model/authentication.dots
    src/model/dotstesttypes.dots
    src/model/daemon.dots
    src/model/legacy.dots
)
target_sources(${TARGET_NAME}
    PRIVATE
        src/Application.cpp
        src/Connection.cpp
        src/Container.cpp
        src/ContainerPool.cpp
        src/Dispatcher.cpp
        src/Event.cpp
        src/GuestTransceiver.cpp
        src/HostTransceiver.cpp
        src/Subscription.cpp
        src/Timer.cpp
        src/Transceiver.cpp
        src/dots.cpp

        src/tools/logging.cpp
        src/tools/IpNetwork.cpp
        src/tools/Uri.cpp

        src/io/Io.cpp
        src/io/Endpoint.cpp
        src/io/FdObserver.cpp

        src/io/Channel.cpp
        src/io/Listener.cpp

        src/serialization/AsciiSerialization.cpp

        src/io/channels/TcpListener.cpp
        src/io/channels/TcpChannel.cpp
        src/io/channels/UdsChannel.cpp
        src/io/channels/UdsListener.cpp

        src/io/channels/WebSocketChannel.cpp
        src/io/channels/WebSocketListener.cpp
        src/io/channels/LocalChannel.cpp
        src/io/channels/LocalListener.cpp


        src/io/services/TimerService.cpp
        src/io/Transmission.cpp

        src/io/DescriptorConverter.cpp

        src/io/auth/Nonce.cpp
        src/io/auth/Digest.cpp
        src/io/auth/AuthManager.cpp
        src/io/auth/LegacyAuthManager.cpp

        src/type/AnyStruct.cpp
        src/type/Chrono.cpp
        src/type/Descriptor.cpp
        src/type/DescriptorMap.cpp
        src/type/DynamicEnum.cpp
        src/type/DynamicStruct.cpp
        src/type/EnumDescriptor.cpp
        src/type/PropertyDescriptor.cpp
        src/type/Registry.cpp
        src/type/Struct.cpp
        src/type/StructDescriptor.cpp
        src/type/TypeVisitor.cpp
        src/type/Uuid.cpp
        src/type/VectorDescriptor.cpp
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
    PUBLIC
        # disable permissive mode to enable alternative logical operators
        # note that this needs to be mandatory to ensure portability
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
        # use standard C++ exception model and assume no exceptions for external C functions
        # note that this needs to be public because MSVC is disadvising from mixing exceptions modes
        $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
        # set Windows SDK to target Windows 10
        $<$<CXX_COMPILER_ID:MSVC>:/DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00>
    PRIVATE
        $<$<CXX_COMPILER_ID:GNU>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:Clang>:$<$<NOT:$<BOOL:${CMAKE_CXX_FLAGS}>>:-Wall -Wextra -Wpedantic -Werror>>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
target_compile_definitions(${TARGET_NAME}
    PUBLIC
        # suppress warning for usage of deprecated Win32 socket functions in Boost
        $<$<PLATFORM_ID:Windows>:_WINSOCK_DEPRECATED_NO_WARNINGS>
        # use constexpr property offsets on supported compilers
        # note that this is currently not compatible with Clang
        $<$<NOT:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:MSVC>>>:DOTS_PROPERTIES_NO_CONSTEXPR_OFFSETS>
    PRIVATE
        # suppress warning for usage of unsafe C runtime functions (e.g. getenv)
        $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
        DOTS_NO_GLOBAL_TRANSCEIVER
)
target_link_libraries(${TARGET_NAME}
    PUBLIC
        Boost::boost
        Boost::program_options
        # link pthread which is required for C++ threads on Linux and OSX
        $<$<OR:$<PLATFORM_ID:Linux>,$<PLATFORM_ID:Darwin>>:pthread>
        # link bcrypt which is required for certain Boost functionality on Windows
        $<$<PLATFORM_ID:Windows>:bcrypt>
    PRIVATE
        $<BUILD_INTERFACE:date::date>
        $<BUILD_INTERFACE:date::date-tz>
        $<BUILD_INTERFACE:PicoSHA2::PicoSHA2>
)
set_target_properties(${TARGET_NAME} PROPERTIES
    SOVERSION 0
)

if (NOT BUILD_DOTS_SHARED)
    bundle_static_library(${TARGET_NAME})
endif()

# install
include(GNUInstallDirs)
set_target_properties(${TARGET_NAME} PROPERTIES EXPORT_NAME ${EXPORT_NAME})
install(TARGETS ${TARGET_NAME}
    EXPORT ${TARGET_NAME}-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT dev
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT dev
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBDIR}
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
    ${${TARGET_NAME}-MODEL_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT dev
)
