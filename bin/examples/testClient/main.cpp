#include <dots/eventloop/Timer.h>
#include <dots/cpp_config.h>
#include <dots/io/Application.h>
#include <dots/io/serialization/AsciiSerialization.h>

#include "DotsTestStruct.dots.h"
#include "DotsTestVectorStruct.dots.h"

class MyClientClass
{
public:
    MyClientClass()
    {
        m_subs = dots::subscribe<DotsTestStruct>(FUN(*this, handleTestStruct));
        pnxs::addTimer(1, FUN(*this, handleTimer));
    }

    ~MyClientClass()
    {
        m_subs.unsubscribe();
    }

    void publishStruct()
    {
        DotsTestStruct ts;

        ts.indKeyfField(1);
        ts._publish();
    }

private:
    void handleTimer()
    {
        LOG_INFO_P("timer expired");
        DotsTestStruct ts;
        ts.indKeyfField(1);
        ts.floatField(m_value++);
        ts._publish();

        DotsTestVectorStruct tvs;
        auto& intList = tvs.intList();
        auto& stringList = tvs.stringList();
        auto& ssList = tvs.subStructList();

        intList.push_back(1);
        stringList.push_back("Hallo");

        DotsTestSubStruct subStruct;
        subStruct.flag1(true);

        ssList.push_back(subStruct);
        tvs._publish();

        if (m_value == 1000) {
            dots::Application::instance()->exit();
        }

        pnxs::addTimer(0.001, FUN(*this, handleTimer));
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
    LOG_INFO_S("Published types (" << dots::PublishedType::allChained().size() << "):")
    for (auto &e : dots::PublishedType::allChained())
    {
        LOG_INFO_S("  " << e->td->typeName());
    }
    LOG_INFO_S("Subscribed types (" << dots::SubscribedType::allChained().size() << "):");
    for (auto &e : dots::SubscribedType::allChained())
    {
        LOG_INFO_S("  " << e->td->typeName());
    }
#endif

    LOG_INFO_S("run mainloop");
    return app.exec();
}

