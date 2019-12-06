#pragma once

#include <dots/cpp_config.h>
#include <dots/type/NewStructDescriptor.h>
#include <dots/functional/signal.h>

namespace dots {

class DynamicTypeReceiver
{
public:
    DynamicTypeReceiver(const std::vector<string>& whiteList  = {});
    ~DynamicTypeReceiver() = default;

    pnxs::Signal<void (const type::NewStructDescriptor<>*)> onNewStruct;

private:
    void emitStruct(const type::NewStructDescriptor<>*);

};

}