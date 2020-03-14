#pragma once
#include <array>
#include <string_view>

namespace dots::type
{
    struct Uuid
    {
        using value_t = std::array<uint8_t, 16>;

        Uuid();
        Uuid(const uint8_t data[16]);
        Uuid(const std::string& value);

        Uuid(const Uuid& other) = default;
		Uuid(Uuid&& other) noexcept = default;
		~Uuid() = default;

		Uuid& operator = (const Uuid& rhs) = default;
		Uuid& operator = (Uuid&& rhs) noexcept = default;

        const value_t& data() const;

        bool operator == (const Uuid&) const;
        bool operator < (const Uuid& rhs) const;
        bool operator != (const Uuid&) const;

        std::string toString() const;
        bool fromString(const std::string_view& value);

        static Uuid Random();

    private:

        std::array<uint8_t, 16> m_data;
    };
}
