if (BUILD_UNIT_TESTS)

add_executable(dots-unittests
        TestAnyContainer.cpp
        TestContainer.cpp
        TestAsciiSerialization.cpp
        TestCborSerialization.cpp
        TestChronoDescriptor.cpp
        TestDistributedTypeId.cpp
        TestEnumDescriptor.cpp
        TestFileStore.cpp
        TestJsonSerialization.cpp
        TestPropertySet.cpp
		TestPropertyProxy.cpp
		TestPropertyProxyVoid.cpp
        TestRecord.cpp
        TestRegistry.cpp
        TestRttrVariantTypes.cpp
        TestStruct.cpp
		TestTProperty.cpp
		TestTStruct.cpp
        TestVectorDescriptor.cpp
        TestJsonStore.cpp
        TestFileReader.cpp
        )

target_link_libraries(dots-unittests dots dots_model_extra gmock gtest gtest_main)

endif()