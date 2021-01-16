#include <dots/type/Uuid.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

namespace dots::type
{
    Uuid::Uuid(const uint8_t data[16])
    {
        std::memcpy(m_data.data(), data, m_data.size());
    }

    Uuid::Uuid(const value_t& data) :
        Uuid(data.data())
    {
        /* do nothing */
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

    Uuid Uuid::FromString(const std::string_view& value)
    {
        auto uuid = boost::uuids::string_generator{}(value.data());
        value_t data;
        std::memcpy(data.data(), uuid.data, data.size());

        return Uuid{ data };
    }

    Uuid Uuid::FromData(const std::string_view& data)
    {
        value_t data_;

        if (data.size() != std::tuple_size<value_t>::value)
        {
            throw std::invalid_argument("data does not have required size: " + std::to_string(data.size()));
        }

        std::memcpy(data_.data(), data.data(), data_.size());

        return Uuid{ data_ };
    }

    Uuid Uuid::Random()
    {
        return Uuid{ boost::uuids::random_generator{}().data };
    }

    std::ostream& operator << (std::ostream& os, const Uuid& uuid)
    {
        os << uuid.toString();
        return os;
    }
}