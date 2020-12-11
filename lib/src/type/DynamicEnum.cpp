#include <dots/type/DynamicEnum.h>

namespace dots::type
{
    Descriptor<DynamicEnum>::Descriptor(std::string name, std::vector<EnumeratorDescriptor<DynamicEnum>> enumeratorDescriptors):
        EnumDescriptor<DynamicEnum>(std::move(name), enumeratorDescriptors)
    {
        /* do nothing */
    }
}