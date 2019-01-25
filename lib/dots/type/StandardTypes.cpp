#include "StandardTypes.h"
#include <cstdint>
#include <dots/type/property_set.h>

namespace dots
{
namespace type
{

template
class StandardTypeDescriptor<bool>;

template
class StandardTypeDescriptor<int8_t>;

template
class StandardTypeDescriptor<int16_t>;

template
class StandardTypeDescriptor<int32_t>;

template
class StandardTypeDescriptor<int64_t>;

template
class StandardTypeDescriptor<uint8_t>;

template
class StandardTypeDescriptor<uint16_t>;

template
class StandardTypeDescriptor<uint32_t>;

template
class StandardTypeDescriptor<uint64_t>;

template
class StandardTypeDescriptor<float>;

template
class StandardTypeDescriptor<double>;

template
class StandardTypeDescriptor<long double>;

template
class StandardTypeDescriptor<std::string>;

template
class StandardTypeDescriptor<dots::property_set>;

template
class StandardTypeDescriptor<void*>;

}
}
