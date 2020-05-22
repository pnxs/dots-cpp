#include <dots/io/auth/Digest.h>
#include <openssl/sha.h>
#include <boost/algorithm/hex.hpp>

namespace dots::io
{
    Digest::Digest(const value_t& value) :
        m_value(value)
    {
        /* do nothing */
    }

     Digest::Digest(std::string_view stringValue) :
        Digest([&]()
        {
            value_t value;
            boost::algorithm::unhex(stringValue, value.begin());

            return value;
        }())
    {
        /* do nothing */
    }

     Digest::Digest(Nonce nonce, std::string_view cnonce, std::string_view userName, std::string_view secret) :
        Digest([&]()
        {
            SHA256_CTX a1;
            init(a1);

            update(a1, userName);
            update(a1, "::");
            update(a1, secret);

            SHA256_CTX response;
            init(response);
            update(response, final(a1));
            update(response, ":");
            update(response, &nonce.value(), sizeof(nonce));
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

    void Digest::init(SHA256_CTX& ctx)
    {
        SHA256_Init(&ctx);
    }

    void Digest::update(SHA256_CTX& ctx, std::string_view data)
    {
        SHA256_Update(&ctx, data.data(), data.size());
    }

    void Digest::update(SHA256_CTX& ctx, const void* data, size_t len)
    {
        SHA256_Update(&ctx, data, len);
    }

    auto Digest::final(SHA256_CTX& ctx) -> value_t
    {
        value_t value = {0};
        SHA256_Final(value.data(), &ctx);

        return value;
    }
}