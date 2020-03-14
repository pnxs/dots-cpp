#include <dots/Uuid.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace dots::type
{
    Uuid::Uuid()
    {
        std::memset(m_data.data(), 0, m_data.size());
    }

    Uuid::Uuid(const uint8_t data[16])
    {
        std::memcpy(m_data.data(), data, m_data.size());
    }

    Uuid::Uuid(const std::string& value)
    {
        if (value.size() == 16)
        {
            std::memcpy(m_data.data(), value.data(), m_data.size());
            return;
        }

        throw std::invalid_argument("invalid stringsize: " + std::to_string(value.size()));
    }

    const Uuid::value_t& Uuid::data() const
    {
        return m_data;
    }

    bool Uuid::operator == (const Uuid& rhs) const
    {
        return m_data == rhs.m_data;
    }

    bool Uuid::operator < (const Uuid& rhs) const
    {
        return m_data < rhs.m_data;
    }

    bool Uuid::operator != (const Uuid& rhs) const
    {
        return !(rhs == *this);
    }

    std::string Uuid::toString() const
    {
        boost::uuids::uuid uuid;
        std::memcpy(uuid.data, m_data.data(), sizeof(uuid.data));
        return boost::uuids::to_string(uuid);
    }

    bool Uuid::fromString(const std::string_view& value)
    {
        auto uuid = boost::uuids::string_generator{}(value.data());
        std::memcpy(m_data.data(), uuid.data, m_data.size());
        return true;
    }

    Uuid Uuid::Random()
    {
        return Uuid{ boost::uuids::random_generator{}().data };
    }
}