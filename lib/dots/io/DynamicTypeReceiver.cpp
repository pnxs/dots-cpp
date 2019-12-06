
#include <dots/type/NewEnumDescriptor.h>
#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"
#include "DynamicTypeReceiver.h"
#include "dots/io/NewRegistry.h"
#include <dots/dots.h>

namespace dots {

DynamicTypeReceiver::DynamicTypeReceiver(const std::vector<std::string> &whiteList)
{
    LOG_DEBUG_S("subscribe for descriptor types");

    dots::subscribe<EnumDescriptorData>([](const EnumDescriptorData::Cbd& cbd) {
    	type::EnumDescriptor<>::createFromEnumDescriptorData(cbd());
    }).discard();

    dots::subscribe<StructDescriptorData>([this](const StructDescriptorData::Cbd& cbd) {
        LOG_DEBUG_S("received StructDescriptorData");
		const type::StructDescriptor<>* descriptor = type::StructDescriptor<>::createFromStructDescriptorData(cbd());
        emitStruct(descriptor);
    }).discard();

    transceiver().connection().requestDescriptors(whiteList);
}

void DynamicTypeReceiver::emitStruct(const type::StructDescriptor<> *sd)
{
    onNewStruct(sd);
}

}
