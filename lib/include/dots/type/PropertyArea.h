// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <dots/type/PropertySet.h>
#include <utility>

namespace dots::type
{
    struct PropertyArea
    {
        constexpr PropertyArea() = default;

        constexpr PropertyArea(const PropertyArea& /*other*/)
        {
            /* do nothing */
        }

        constexpr PropertyArea(PropertyArea&& /*other*/) noexcept
        {
            /* do nothing */
        }

        ~PropertyArea() = default;

        constexpr PropertyArea& operator = (const PropertyArea& /*rhs*/)
        {
            return *this;
        }

        constexpr PropertyArea& operator = (PropertyArea&& /*rhs*/) noexcept
        {
            return *this;
        }

        constexpr const PropertySet& validProperties() const
        {
            return m_validProperties;
        }

        constexpr PropertySet& validProperties()
        {
            return m_validProperties;
        }

        template <typename P>
        const P& getProperty(size_t offset) const
        {
            return *reinterpret_cast<const P*>(reinterpret_cast<const char*>(this) + offset);
        }

        template <typename P>
        P& getProperty(size_t offset)
        {
            return const_cast<P&>(std::as_const(*this).getProperty<P>(offset));
        }

        template <typename P>
        const P& getProperty() const
        {
            size_t offset = P::Offset();
            return getProperty<P>(offset);
        }

        template <typename P>
        P& getProperty()
        {
            return const_cast<P&>(std::as_const(*this).getProperty<P>());
        }

        template <typename P>
        static const PropertyArea& GetArea(const P& property, size_t offset)
        {
            return *reinterpret_cast<const PropertyArea*>(reinterpret_cast<const char*>(&property) - offset);
        }

        template <typename P>
        static PropertyArea& GetArea(P& property, size_t offset)
        {
            return const_cast<PropertyArea&>(GetArea<P>(std::as_const(property), offset));
        }

        template <typename P>
        static const PropertyArea& GetArea(const P& property)
        {
            size_t offset = P::Offset();
            return GetArea(property, offset);
        }

        template <typename P>
        static PropertyArea& GetArea(P& property)
        {
            return const_cast<PropertyArea&>(GetArea(std::as_const(property)));
        }

    private:

        PropertySet m_validProperties;
    };
}
