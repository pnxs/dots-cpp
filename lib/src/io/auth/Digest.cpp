#include <dots/io/auth/Digest.h>
#if (defined _MSC_VER)
// suppress warnings for implicit 'possible loss of data' conversions
#pragma warning(push)
#pragma warning(disable: 4244)
#include <PicoSHA2/picosha2.h>
#pragma warning(pop)
#else
#include <PicoSHA2/picosha2.h>
#endif
#include <boost/algorithm/hex.hpp>

namespace dots::io
{
    static_assert(std::tuple_size<Digest::value_t>::value == picosha2::k_digest_size);

    Digest::Digest(const value_t& value) :
        m_value(value)
    {
        /* do nothing */
    }

     Digest::Digest(std::string_view stringValue) :
        Digest([&]()
        {
            value_t value = {};
            boost::algorithm::unhex(stringValue, value.begin());

            return value;
        }())
    {
        /* do nothing */
    }

     Digest::Digest(Nonce nonce, std::string_view cnonce, std::string_view userName, std::string_view secret) :
        Digest([&]()
        {
            picosha2::hash256_one_by_one a1;
            update(a1, userName);
            update(a1, "::");
            update(a1, secret);

            picosha2::hash256_one_by_one response;
            update(response, final(a1));
            update(response, ":");
            update(response, reinterpret_cast<const uint8_t*>(&nonce.value()), sizeof(nonce));
            update(response, ":");
            update(response, cnonce);

            return final(response);
        }())
    {
        /* do nothing */
    }

    Digest::Digest(Nonce nonce, Nonce cnonce, std::string_view userName, std::string_view secret) :
        Digest(nonce, cnonce.toString(), userName, secret)
    {
        /* do nothing */
    }

    auto Digest::value() const -> const value_t&
    {
        return m_value;
    }

    std::string Digest::toString() const
    {
        return boost::algorithm::hex_lower(std::string(m_value.begin(), m_value.end()));
    }

    void Digest::update(picosha2::hash256_one_by_one& hasher, std::string_view data)
    {
        hasher.process(data.begin(), data.end());
    }

    void Digest::update(picosha2::hash256_one_by_one& hasher, const uint8_t* data, size_t len)
    {
        hasher.process(data, data + len);
    }

    auto Digest::final(picosha2::hash256_one_by_one& hasher) -> value_t
    {
        hasher.finish();
        value_t value = {0};
        hasher.get_hash_bytes(value.begin(), value.end());

        return value;
    }
}