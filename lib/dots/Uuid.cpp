#include "Uuid.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace dots {

dots::uuid dots::uuid::generateRandom()
{
    boost::uuids::uuid newUuid = boost::uuids::random_generator()();
    return uuid(newUuid.data);
}

uuid::uuid(const uint8_t data[16])
{
    memcpy(m_data.data(), data, m_data.size());
}

uuid::uuid()
{
    memset(m_data.data(), 0, m_data.size());
}

bool uuid::operator==(const uuid &rhs) const
{
    return m_data == rhs.m_data;
}

bool uuid::operator<(const uuid &rhs) const
{
    return m_data < rhs.m_data;
}

bool uuid::operator!=(const uuid &rhs) const
{
    return !(rhs == *this);
}

uuid::uuid(const std::string &strdata)
{
    if (strdata.size() == 16)
    {
        memcpy(m_data.data(), strdata.data(), m_data.size());
        return;
    }

    throw std::runtime_error("invalid stringsize: " + std::to_string(strdata.size()));
}

std::string uuid::toString() const
{
    boost::uuids::uuid boostUuid;
    memcpy(boostUuid.data, m_data.data(), sizeof(boostUuid.data));
    return boost::uuids::to_string(boostUuid);
}

bool uuid::fromString(const std::string &str)
{
    auto boostUuid = boost::uuids::string_generator()(str);
    memcpy(m_data.data(), boostUuid.data, m_data.size());
    return true;
}


}
