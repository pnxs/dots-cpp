#pragma once

#include <dots/type/StructDescriptor.h>
#include <dots/functional/signal.h>

namespace dots {

class DynamicTypeReceiver
{
public:
    DynamicTypeReceiver(const std::vector<std::string>& whiteList  = {});
    ~DynamicTypeReceiver();

    pnxs::Signal<void (const type::StructDescriptor<>*)> onNewStruct;

private:
    void emitStruct(const type::StructDescriptor<>*);

    pnxs::SignalConnection m_structObserverConnection;
};

}