#include <string>
#include <dots/common/Chrono.h>
#include "Registry.h"
#include "dots/common/logging.h"

#include "StructDescriptor.h"
#include "EnumDescriptor.h"
#include "VectorDescriptor.h"

//#include <iostream>

using namespace std::string_literals;

namespace dots
{
namespace type
{

//std::once_flag Registry::m_populated;
std::atomic_flag Registry::m_populated;

Registry::Registry()
{
}

void Registry::populate_standard_types()
{
    get<bool>("bool", DotsType::boolean);
    get<int8_t>("int8", DotsType::int8);
    get<int16_t>("int16", DotsType::int16);
    get<int32_t>("int32", DotsType::int32);
    get<int64_t>("int64", DotsType::int64);
    get<uint8_t>("uint8", DotsType::uint8);
    get<uint16_t>("uint16", DotsType::uint16);
    get<uint32_t>("uint32", DotsType::uint32);
    get<uint64_t>("uint64", DotsType::uint64);
    get<float>("float32", DotsType::float32);
    get<double>("float64", DotsType::float64);
    //get<long double>("float128", DotsType::float128);

    get<TimePoint>("timepoint");
    get<SteadyTimePoint>("steady_timepoint");
    get<Duration>("duration");

    get<std::string>("string", DotsType::string);
    get<dots::property_set>("property_set", DotsType::property_set);
    get<void*>("pointer", DotsType::pointer);

    get<dots::uuid>("uuid");
}

const Descriptor *Registry::findDescriptor(const std::string &name) const
{
    auto iter = m_types.find(name);
    return iter != m_types.end() ? iter->second : nullptr;
}

const StructDescriptor* Registry::findStructDescriptor(const std::string& name) const
{
    auto td = findDescriptor(name);
    return dynamic_cast<const StructDescriptor*>(td);
}

void Registry::checkPopulate()
{
    if (not m_populated.test_and_set())
    {
        this->populate_standard_types();
    }
}

void Registry::clear()
{
    for (auto& e : m_types)
    {
        delete e.second;
    }
    m_types.clear();
    m_populated.clear();
}

Registry::~Registry()
{
    clear();
}

const Descriptor* Registry::fromWireName(const std::string &type_name)
{
    auto& registry = Descriptor::registry();

    if (type_name.find("vector<") == 0)
    {
        auto vectorTypeName = type_name.substr(7, type_name.size()-8); // remove "vector<" from string

        // check if vector-descriptor is already registered
        auto desc = registry.findDescriptor(type_name);
        if (not desc)
        {
            desc = VectorDescriptor::createDescriptor(vectorTypeName);
        }

        return desc;
    }

    auto desc = registry.findDescriptor(type_name);
    return desc;
}


void Registry::insertType(const std::string &name, const Descriptor *descriptor)
{
    LOG_DEBUG_S("insert type '" << name << "'");
    auto ret = m_types.insert({name, descriptor});
    if (ret.second == false) {
        throw std::runtime_error("type " + name + " is already registered");
    }
    onNewType(descriptor);
}

void Registry::removeType(const std::string &name)
{
    m_types.erase(name);
}

}
}
