
#include <dots/type/EnumDescriptor.h>
#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"
#include "DynamicTypeReceiver.h"
#include "dots/type/Registry.h"
#include <dots/dots.h>

namespace dots {

DynamicTypeReceiver::DynamicTypeReceiver(const std::vector<std::string> &whiteList)
{
    auto& registry = type::Descriptor::registry();
    m_structObserverConnection = registry.onNewStruct.connect(FUN(*this, emitStruct));

    LOG_DEBUG_S("subscribe for descriptor types");

    dots::subscribe<EnumDescriptorData>([](const EnumDescriptorData::Cbd& cbd) {
        type::EnumDescriptor::createFromEnumDescriptorData(cbd());
    }).discard();

    dots::subscribe<StructDescriptorData>([](const StructDescriptorData::Cbd& cbd) {
        LOG_DEBUG_S("received StructDescriptorData");
        type::StructDescriptor::createFromStructDescriptorData(cbd());
    }).discard();

    transceiver().connection().requestDescriptors(whiteList);
}

DynamicTypeReceiver::~DynamicTypeReceiver()
{
    m_structObserverConnection.disconnect();
}

void DynamicTypeReceiver::emitStruct(const type::StructDescriptor *sd)
{
    onNewStruct(sd);
}

}
