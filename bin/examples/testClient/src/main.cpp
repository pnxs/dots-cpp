#include <dots/Application.h>
#include <dots/tools/logging.h>
#include <dots/io/serialization/AsciiSerialization.h>

#include "DotsTestStruct.dots.h"
#include "DotsTestVectorStruct.dots.h"
#include <dots/tools/fun.h>

using namespace dots::types::literals;

class MyClientClass
{
public:
    MyClientClass() :
        m_subs{ dots::subscribe<DotsTestStruct>(FUN(*this, handleTestStruct)) }
    {
        dots::add_timer(1s, FUN(*this, handleTimer));
    }

    ~MyClientClass()
    {
        m_subs.unsubscribe();
    }

    void publishStruct()
    {
        DotsTestStruct ts;

        ts.indKeyfField(1);
        dots::publish(ts);
    }

private:
    void handleTimer()
    {
        LOG_INFO_P("timer expired");
        DotsTestStruct ts;
        ts.indKeyfField(1);
        ts.floatField(static_cast<float>(m_value++));
        dots::publish(ts);

        DotsTestVectorStruct tvs;
        auto& intList = tvs.intList();
        auto& stringList = tvs.stringList();
        auto& ssList = tvs.subStructList();

        intList.push_back(1);
        stringList.push_back("Hallo");

        DotsTestSubStruct subStruct;
        subStruct.flag1(true);

        ssList.push_back(subStruct);
        dots::publish(tvs);

        if (m_value == 1000) {
            dots::Application::instance()->exit();
        }

        dots::add_timer(1ms, FUN(*this, handleTimer));
    }

    void handleTestStruct(const DotsTestStruct::Cbd& cbd)
    {
        //TODO: implement replacement for toString()
        LOG_INFO_S("received subscribed TestStruct:" << dots::to_ascii(&cbd()._Descriptor(), &cbd()));
    }

    dots::Subscription m_subs;

    int m_value = 0;

};

int main(int argc, char *argv[])
{
    LOG_INFO_S("start and connect");
    dots::Application app("testClient", argc, argv);

    MyClientClass mcc;

    LOG_INFO_S("connected, publish struct");
    mcc.publishStruct();

#if 0
    LOG_INFO_S("Published types (" << dots::io::global_publish_types().size() << "):")
    for (const dots::type::StructDescriptor<>& descriptor : dots::io::global_publish_types())
    {
        LOG_INFO_S("  " << descriptor.name());
    }
    LOG_INFO_S("Subscribed types (" << dots::io::global_subscribe_types().size() << "):");
    for (const dots::type::StructDescriptor<>& descriptor : dots::io::global_subscribe_types())
    {
        LOG_INFO_S("  " << descriptor.name());
    }
#endif

    LOG_INFO_S("run mainloop");
    return app.exec();
}

