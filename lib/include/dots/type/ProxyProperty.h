// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#include <variant>
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#include <dots/type/Property.h>
#include <dots/type/PropertyPath.h>

namespace dots::type
{
    template <typename T = Typeless>
    struct ProxyProperty : Property<T, ProxyProperty<T>>
    {
        static constexpr bool EnableValueConstructors = !std::is_same_v<T, PropertyArea>;

        template <typename Derived>
        static constexpr bool is_compatible_property_v = std::conjunction_v<std::negation<std::integral_constant<bool, Property<T, ProxyProperty<T>>::IsTypeless>>, std::integral_constant<bool, !std::is_same_v<T, PropertyArea>>>;

        template <typename Derived>
        static constexpr bool is_compatible_typeless_property_v = std::conjunction_v<std::integral_constant<bool, Property<T, ProxyProperty<T>>::IsTypeless>, std::integral_constant<bool, EnableValueConstructors>>;

        ProxyProperty(const PropertyPath& path) :
            ProxyProperty(nullptr, path_t{ &path })
        {
            /* do nothing */
        }

        ProxyProperty(PropertyArea& area, const PropertyPath& path) :
            ProxyProperty(&area, path_t{ &path })
        {
            /* do nothing */
        }

        ProxyProperty(PropertyArea& area, const PropertyDescriptor& descriptor) :
            ProxyProperty(&area, path_t{ &descriptor })
        {
            /* do nothing */
        }

        template <bool C = EnableValueConstructors, std::enable_if_t<C, int> = 0>
        ProxyProperty(T& value, const PropertyDescriptor& descriptor) :
            ProxyProperty(PropertyArea::GetArea<T>(value, descriptor.offset()), descriptor)
        {
            /* do nothing */
        }

        template <typename Derived, std::enable_if_t<is_compatible_property_v<Derived>, int> = 0>
        ProxyProperty(Property<T, Derived>& property) :
            ProxyProperty(property.storage(), property.descriptor())
        {
            /* do nothing */
        }

        template <typename U, typename Derived, std::enable_if_t<is_compatible_typeless_property_v<Derived>, int> = 0>
        ProxyProperty(Property<U, Derived>& property) :
            ProxyProperty(Typeless::From(property.storage()), property.descriptor())
        {
            /* do nothing */
        }

        using Property<T, ProxyProperty<T>>::operator=;

        template <typename U>
        bool is() const
        {
            using descriptor_t = Descriptor<U>;
            static_assert(!is_dynamic_descriptor_v<descriptor_t>);

            return &descriptor_t::Instance() == &derivedDescriptor().valueDescriptor();
        }

        template <typename U>
        const ProxyProperty<U>* as() const
        {
            return is<U>() ? reinterpret_cast<const ProxyProperty<U>*>(this) : nullptr;
        }

        template <typename U>
        const ProxyProperty<U>* as()
        {
            return const_cast<ProxyProperty<U>*>(std::as_const(*this).template as<U>());
        }

        template <typename U, bool Safe = false>
        const ProxyProperty<U>& to() const
        {
            using descriptor_t = Descriptor<U>;
            static_assert(!is_dynamic_descriptor_v<descriptor_t>);

            if constexpr (Safe)
            {
                if (!is<U>())
                {
                    throw std::logic_error{ std::string{ "type mismatch in safe ProxyProperty conversion: expected '" } + derivedDescriptor().valueDescriptor().name() + "' but got '" + descriptor_t::Instance().name() + "'" };
                }
            }

            return reinterpret_cast<const ProxyProperty<U>&>(*this);
        }

        template <typename U, bool Safe = false>
        ProxyProperty<U>& to()
        {
            return const_cast<ProxyProperty<U>&>(std::as_const(*this).template to<U, Safe>());
        }

    private:

        friend struct Property<T, ProxyProperty<T>>;
        using path_t = std::variant<const PropertyDescriptor*, const PropertyPath*>;
        static constexpr PropertySet None{ PropertySet::None };

        ProxyProperty(PropertyArea* area, path_t path) :
            m_area(area),
            m_path{ std::move(path) }
        {
            /* do nothing */
        }

        PropertySet validPathProperties() const
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return m_area->validProperties();
            }
            else
            {
                if (const auto& elements = std::get<const PropertyPath*>(m_path)->elements(); elements.size() == 1)
                {
                    return m_area->validProperties();
                }
                else
                {
                    PropertyArea* area = m_area;

                    for (size_t i = 0; i < elements.size() - 1; ++i)
                    {
                        ProxyProperty<> subProperty{ *area, elements[i].get() };

                        if (subProperty.isValid())
                        {
                            area = reinterpret_cast<PropertyArea*>(reinterpret_cast<std::byte*>(&subProperty.storage()) + *subProperty.descriptor().subAreaOffset());
                        }
                        else
                        {
                            return None;
                        }
                    }

                    return area->validProperties();
                }
            }
        }

        PropertySet& validPathProperties()
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return m_area->validProperties();
            }
            else
            {
                if (const auto& elements = std::get<const PropertyPath*>(m_path)->elements(); elements.size() == 1)
                {
                    return m_area->validProperties();
                }
                else
                {
                    PropertyArea* area = m_area;

                    for (size_t i = 0; i < elements.size() - 1; ++i)
                    {
                        ProxyProperty<> subProperty{ *area, elements[i].get() };
                        area = reinterpret_cast<PropertyArea*>(reinterpret_cast<std::byte*>(&subProperty.valueOrEmplace()) + *subProperty.descriptor().subAreaOffset());
                    }

                    return area->validProperties();
                }
            }
        }

        const T& derivedStorage() const
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return m_area->getProperty<T>(std::get<const PropertyDescriptor*>(m_path)->offset());
            }
            else
            {
                return m_area->getProperty<T>(std::get<const PropertyPath*>(m_path)->offset());
            }
        }

        const PropertyDescriptor& derivedDescriptor() const
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return *std::get<const PropertyDescriptor*>(m_path);
            }
            else
            {
                return std::get<const PropertyPath*>(m_path)->elements().back();
            }
        }

        bool derivedIsValid() const
        {
            return derivedDescriptor().set() <= validPathProperties();
        }

        void derivedSetValid()
        {
            validPathProperties() += derivedDescriptor().set();
        }

        void derivedSetInvalid()
        {
            validPathProperties() -= derivedDescriptor().set();
        }

        PropertyArea* m_area;
        path_t m_path;
    };
}

#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
