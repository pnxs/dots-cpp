#pragma once
#include <variant>
#include <dots/type/Property.h>

namespace dots::type
{
    template <typename T = Typeless>
    struct ProxyProperty : Property<T, ProxyProperty<T>>
    {
        static constexpr bool EnableValueConstructors = !std::is_same_v<T, PropertyArea>;

        ProxyProperty(PropertyArea& area, const property_path_t& path) :
            m_area(&area),
            m_path{ &path },
            m_offset(determineOffset())
        {
            /* do nothing */
        }

        ProxyProperty(PropertyArea& area, const PropertyDescriptor& descriptor) :
            m_area(&area),
            m_path{ &descriptor },
            m_offset(determineOffset())
        {
            /* do nothing */
        }

        template <std::enable_if_t<EnableValueConstructors, int> = 0>
        ProxyProperty(T& value, const PropertyDescriptor& descriptor) :
            ProxyProperty(PropertyArea::GetArea<T>(value, descriptor.offset()), descriptor)
        {
            /* do nothing */
        }

        template <typename Derived, std::enable_if_t<EnableValueConstructors && !Property<T, ProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, ProxyProperty<T>>, int> = 0>
        ProxyProperty(Property<T, Derived>& property) :
            ProxyProperty(property.storage(), property.descriptor())
        {
            /* do nothing */
        }

        template <typename U, typename Derived, std::enable_if_t<EnableValueConstructors && Property<T, ProxyProperty<T>>::IsTypeless && !std::is_same_v<Derived, ProxyProperty<T>>, int> = 0>
        ProxyProperty(Property<U, Derived>& property) :
            ProxyProperty(Typeless::From(property.storage()), property.descriptor())
        {
            /* do nothing */
        }
        ProxyProperty(const ProxyProperty& other) = default;
        ProxyProperty(ProxyProperty&& other) = default;
        ~ProxyProperty() = default;

        ProxyProperty& operator = (const ProxyProperty& rhs) = default;
        ProxyProperty& operator = (ProxyProperty&& rhs) = default;

        const PropertyArea& area() const
        {
            return *m_area;
        }

        void area(PropertyArea& area)
        {
            m_area = &area;
        }

        void area(PropertyArea& area, const property_path_t& path)
        {
            m_area = &area;
            m_path = &path;
            m_offset = determineOffset();
        }

        template <typename U>
        bool is() const
        {
            using descriptor_t = Descriptor<U>;
            static_assert(!is_dynamic_descriptor_v<descriptor_t>);

            return descriptor_t::InstancePtr() == derivedDescriptor().valueDescriptorPtr();
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
        using path_t = std::variant<const PropertyDescriptor*, const property_path_t*>;
        static constexpr PropertySet None{ PropertySet::None };

        const PropertySet& validPathProperties() const
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return m_area->validProperties();
            }
            else
            {
                if (const auto& path = *std::get<const property_path_t*>(m_path); path.size() == 1)
                {
                    return m_area->validProperties();
                }
                else
                {
                    PropertyArea* area = m_area;

                    for (size_t i = 0; i < path.size() - 1; ++i)
                    {
                        ProxyProperty<> subProperty{ *area, path[i].get() };

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
                if (const auto& path = *std::get<const property_path_t*>(m_path); path.size() == 1)
                {
                    return m_area->validProperties();
                }
                else
                {
                    PropertyArea* area = m_area;

                    for (size_t i = 0; i < path.size() - 1; ++i)
                    {
                        ProxyProperty<> subProperty{ *area, path[i].get() };
                        area = reinterpret_cast<PropertyArea*>(reinterpret_cast<std::byte*>(&subProperty.constructOrValue()) + *subProperty.descriptor().subAreaOffset());
                    }

                    return area->validProperties();
                }
            }
        }

        size_t determineOffset()
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return std::get<const PropertyDescriptor*>(m_path)->offset();
            }
            else
            {
                size_t offset = 0;
                const auto& path = *std::get<const property_path_t*>(m_path);

                for (size_t i = 0; i < path.size() - 1; ++i)
                {
                    const PropertyDescriptor& descriptor = path[i];
                    offset += descriptor.offset();
                    offset += *descriptor.subAreaOffset();
                }

                offset += path[path.size() - 1].get().offset();
                return offset;
            }
        }

        const PropertySet& derivedValidProperties() const
        {
            static_assert(!std::is_same_v<T, T>, "derivedValidProperties shall not be used");
            return None;
        }

        PropertySet& derivedValidProperties()
        {
            return const_cast<PropertySet&>(std::as_const(*this).derivedValidProperties());
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

        T& derivedStorage()
        {
            return m_area->getProperty<T>(m_offset);
        }

        const T& derivedStorage() const
        {
            return const_cast<ProxyProperty&>(*this).derivedValue();
        }

        const PropertyDescriptor& derivedDescriptor() const
        {
            if (std::holds_alternative<const PropertyDescriptor*>(m_path))
            {
                return *std::get<const PropertyDescriptor*>(m_path);
            }
            else
            {
                return std::get<const property_path_t*>(m_path)->back();
            }
        }

        PropertyArea* m_area;
        path_t m_path;
        size_t m_offset;
    };
}