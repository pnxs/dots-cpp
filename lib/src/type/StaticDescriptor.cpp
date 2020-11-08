#include <dots/type/StaticDescriptor.h>

namespace dots::type
{
    std::shared_ptr<Descriptor<>> StaticDescriptorMap::Emplace(std::shared_ptr<Descriptor<>> descriptor)
    {
        auto [it, emplaced] = DescriptorsMutable().try_emplace(descriptor->name(), descriptor);

        if (!emplaced)
        {
            throw std::logic_error{ "there already is a static descriptor with name: " + descriptor->name() };
        }

        return descriptor;
    }
}