#pragma once

#include <dots/cpp_config.h>
#include <dots/type/StructDescriptor.h>
#include <dots/functional/signal.h>

namespace dots {

class DynamicTypeReceiver
{
public:
    DynamicTypeReceiver(const std::vector<string>& whiteList  = {});
    ~DynamicTypeReceiver();

    pnxs::Signal<void (const type::StructDescriptor<>*)> onNewStruct;

private:
    void emitStruct(const type::StructDescriptor<>*);

    pnxs::SignalConnection m_structObserverConnection;
};

}