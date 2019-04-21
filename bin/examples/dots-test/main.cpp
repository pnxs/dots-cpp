#include <dots/cpp_config.h>
#include <dots/io/Application.h>
#include <dots/io/serialization/CborNativeSerialization.h>

#include "DotsTestStruct.dots.h"
#include "DotsClearCache.dots.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
using std::string;

DotsTestStruct createTestData()
{
    DotsTestStruct dts;
    dts.indKeyfField(42);
    dts.stringField("Hello");
    dts.enumField(DotsTestEnum::value2);
    dts.floatField(3.141);
    dts.tp(pnxs::TimePoint(1234));
    auto& s = dts.subStruct();
    s.flag1(true);

    return dts;
}

void publish_test(int loop_count)
{
    for (int i = 0; i < loop_count; ++i)
    {
        DotsTestStruct dts = createTestData();
        dts._publish();
    }
}

void serializeStruct(const DotsTestStruct& dts)
{
    dots::to_cbor(dts);
}

extern "C" {
void serializeTest(int loop_count)
{
    DotsTestStruct dts = createTestData();
    for (int i = 0; i < loop_count; ++i)
    {
        serializeStruct(dts);
    }
}
}

void clearCache(const string& type)
{
    DotsClearCache cc;
    auto& types = cc.typeNames();
    types.push_back(type);

    cc._publish();
}

extern "C" {
void costExperiment() {
    int i = 0;



    for (i = 0; i < 10; ++i) {
    }

    printf("I: %d", i);

}
}



int main(int argc, char *argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h", "display help message")
            ("count", po::value<int>()->default_value(10), "nr of loops")
            ("publish", "publish-test")
            ("serialize", "publish-test")
            ("experiment", "experiment")
            ("clear-cache", po::value<string>(), "clear cache for type")
            ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    dots::Application app("dots-test", argc, argv);

    if (vm.count("publish")) {
        auto count = vm["count"].as<int>();
        publish_test(count);
    }

    if (vm.count("serialize")) {
        auto count = vm["count"].as<int>();
        serializeTest(count);
    }

    if (vm.count("clear-cache")) {
        clearCache(vm["clear-cache"].as<string>());
    }

    if (vm.count("experiment")) {
        costExperiment();
    }

    //return app.exec();
}