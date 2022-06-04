#include <dots/type/DynamicEnum.h>

namespace dots::type
{
    Descriptor<DynamicEnum>::Descriptor(key_t key, std::string name, std::vector<EnumeratorDescriptor> enumeratorDescriptors):
        EnumDescriptor<>(key, std::move(name), std::move(enumeratorDescriptors))
    {
        /* do nothing */
    }
}
