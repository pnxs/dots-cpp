
#include <dots/type/EnumDescriptor.h>
#include "EnumDescriptorData.dots.h"
#include "StructDescriptorData.dots.h"
#include "DynamicTypeReceiver.h"
#include "dots/io/Registry.h"
#include <dots/dots.h>
#include "DotsDescriptorRequest.dots.h"

namespace dots {

DynamicTypeReceiver::DynamicTypeReceiver(const std::vector<std::string> &whiteList)
{
    auto& registry = transceiver().registry();
    m_structObserverConnection = registry.onNewStruct.connect(FUN(*this, emitStruct));

    LOG_DEBUG_S("subscribe for descriptor types");

    dots::subscribe<EnumDescriptorData>([](const EnumDescriptorData::Cbd& cbd) {
        type::EnumDescriptor<>::createFromEnumDescriptorData(cbd());
    }).discard();

    dots::subscribe<StructDescriptorData>([](const StructDescriptorData::Cbd& cbd) {
        LOG_DEBUG_S("received StructDescriptorData");
        type::StructDescriptor<>::createFromStructDescriptorData(cbd());
    }).discard();

    DotsDescriptorRequest request;

	if (!whiteList.empty())
	{
		request.whitelist();
		
		for (auto& e : whiteList)
		{
			request.whitelist->push_back(e);
		}
	}

	request._publish();
}

DynamicTypeReceiver::~DynamicTypeReceiver()
{
    m_structObserverConnection.disconnect();
}

void DynamicTypeReceiver::emitStruct(const type::StructDescriptor<>* sd)
{
    onNewStruct(sd);
}

}
