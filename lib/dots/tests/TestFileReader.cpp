#include "dots/io/recording/FileReader.h"

#include "DotsTestStruct.dots.h"
#include "DotsRecordHeader.dots.h"
#include "dots/type/Registry.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fstream>
using namespace dots::type;

TEST(TestFileReader, read)
{
    dots::FileReader reader("/home/thomas/Downloads/HMITest.dots.rec");

    reader.scan();

    LOG_INFO_S("Record duration: " << reader.totalRecordDuration())
    LOG_INFO_S("Nr. of Records: " << reader.totalRecords());
    
    LOG_INFO_S("Start timepoint: " << reader.startTimePoint().toString());

    for (auto& event : reader.recordEvents())
    {
        LOG_INFO_S(" Events of " << event.first << ":" << event.second);
    }

    reader.registerDescriptors();

    uint32_t event_count = 0;

    //std::ofstream plotfile("/tmp/test.plot");

    for (const auto& item : reader)
    {
        //plotfile << std::setprecision(std::numeric_limits<double>::digits10 + 1) << item.header.sentTime().value() << " 10\n";
        event_count++;
    }

    EXPECT_EQ(event_count, reader.totalRecords());

}