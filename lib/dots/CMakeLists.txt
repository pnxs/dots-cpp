

if (WITH_QT5)
    find_package(Qt5 COMPONENTS Core Network Widgets REQUIRED)

    set(CMAKE_AUTOMOC ON)
    #set(CMAKE_AUTOUIC ON)
endif()

add_library(dots SHARED
        dots_base.cpp
        Uuid.cpp
        common/logging.cpp
        common/Stream.cpp
        eventloop/AsioTimer.cpp
        eventloop/Chrono.cpp
        eventloop/event_loop.cpp
        eventloop/Io.cpp
        eventloop/IoService.cpp
        eventloop/seconds.cpp
        eventloop/string_convert.cpp
        eventloop/Timer.cpp
        functional/signal.cpp

        io/Socket.cpp
        io/TcpSocket.cpp
        io/ConstBufSeq.cpp
        io/TcpEndpoint.cpp
        io/TcpAcceptor.cpp
        io/Transmitter.cpp
        io/Transceiver.cpp
        io/ServerConnection.cpp
        io/TcpResolver.cpp
        io/Subscription.cpp
        io/Chained.cpp
        io/DotsSocket.cpp
        io/DotsAsioSocket.cpp

        io/AnyContainer.cpp
        io/Container.cpp
        io/Dispatcher.cpp
        io/TD_Traversal.cpp
        io/Application.cpp
        io/DynamicTypeReceiver.cpp
        io/ResourceUsage.cpp
        io/Message.cpp
        io/DistributedTypeId.cpp
        io/Filter.cpp
        io/serialization/JsonSerialization.cpp
        io/serialization/AsciiSerialization.cpp
        io/serialization/CborNativeSerialization.cpp
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
        type/ChronoDescriptor.cpp
        type/Descriptor.cpp
        type/EnumDescriptor.cpp
        type/Registry.cpp
        type/StandardTypes.cpp
        type/StructDescriptor.cpp
        type/StructDescriptorConstruct.cpp
        type/StructProperty.cpp
        type/UuidDescriptor.cpp
        type/Vector.cpp
        type/VectorDescriptor.cpp
		type/Struct.cpp
)

if (WITH_QT5)
target_sources(dots
        PRIVATE
        io/QtCoreApplication.cpp
        io/DotsQtSocket.cpp
        qt5/QtSingleShotTimer.cpp
        )
endif()

set_target_properties(dots
        PROPERTIES
            SOVERSION 0)

add_subdirectory(model)

target_include_directories(dots
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS dots EXPORT dotsConfig
        DESTINATION lib
)

install(DIRECTORY .
        DESTINATION include/dots
        FILES_MATCHING PATTERN "*.h"
)

target_include_directories(dots
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:dots-model,INTERFACE_INCLUDE_DIRECTORIES>>
        #$<INSTALL_INTERFACE:include>
)

target_sources(dots
    PRIVATE
        $<TARGET_OBJECTS:dots-model>
)


target_link_libraries(dots
        PRIVATE
            rapidjson cborcpp Boost::iostreams Boost::program_options Boost::system
        PUBLIC
            Boost::boost
            pthread)

if (WITH_QT5)
    target_link_libraries(dots
            PRIVATE
            Qt5::Core
            PUBLIC
            Qt5::Network Qt5::Widgets
            )
endif()

install(EXPORT dotsConfig DESTINATION share/dots)
#export(TARGETS dots FILE dotsConfig.cmake)

add_subdirectory(tests)