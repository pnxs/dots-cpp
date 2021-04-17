#pragma once
#include <type_traits>
#include <optional>
#include <dots/type/Property.h>
#include <dots/type/StaticPropertyMetadata.h>

namespace dots::type
{
    template <typename T, typename Derived>
    struct StaticProperty : Property<T, Derived>
    {
        using Property<T, Derived>::Property;
        using Property<T, Derived>::operator=;

        static constexpr std::string_view Name()
        {
            return Derived::Metadata.name();
        }

        static constexpr uint32_t Tag()
        {
            return Derived::Metadata.tag();
        }

        static constexpr bool IsKey()
        {
            return Derived::Metadata.isKey();
        }

        static constexpr PropertyOffset Offset()
        {
            return Derived::Metadata.offset();
        }

        static constexpr PropertySet Set()
        {
            return Derived::Metadata.set();
        }

        static constexpr bool IsPartOf(const PropertySet& propertySet)
        {
            return Set() <= propertySet;
        }

        static PropertyDescriptor InitDescriptor() 
        {
            if (M_descriptorStorage == std::nullopt)
            {
                M_descriptorStorage.emplace(type::Descriptor<T>::Instance(), Name().data(), Tag(), IsKey(), Offset());
            }

            return *M_descriptorStorage; 
        }

        inline static const type::PropertyDescriptor& Descriptor = InitDescriptor();

    protected:

        StaticProperty() = default;

        StaticProperty(const StaticProperty& other) : Property<T, Derived>()
        {
            if (other.isValid())
            {
                Property<T, Derived>::template construct<false>(static_cast<const Derived&>(other));
            }
        }

        StaticProperty(StaticProperty&& other)
        {
            if (other.isValid())
            {
                Property<T, Derived>::template construct<false>(static_cast<Derived&&>(other));
            }
        }

        ~StaticProperty()
        {
            Property<T, Derived>::destroy();
        }

        StaticProperty& operator = (const StaticProperty& rhs)
        {
            if (rhs.isValid())
            {
                Property<T, Derived>::constructOrAssign(static_cast<const Derived&>(rhs));
            }
            else
            {
                Property<T, Derived>::destroy();
            }

            return *this;
        }

        StaticProperty& operator = (StaticProperty&& rhs)
        {
            if (rhs.isValid())
            {
                Property<T, Derived>::constructOrAssign(static_cast<Derived&&>(rhs));
            }
            else
            {
                Property<T, Derived>::destroy();
            }

            return *this;
        }

    private:

        friend struct Property<T, Derived>;

        T& derivedStorage()
        {
            return reinterpret_cast<T&>(m_storage);
        }

        const T& derivedStorage() const
        {
            return const_cast<StaticProperty&>(*this).derivedStorage();
        }

        static constexpr size_t derivedOffset()
        {
            return Derived::Offset;
        }

        static const PropertyDescriptor& derivedDescriptor()
        {
            return Derived::Descriptor;
        }

        const PropertySet& derivedValidProperties() const
        {
            return PropertyArea::GetArea(static_cast<const Derived&>(*this)).validProperties();
        }

        PropertySet& derivedValidProperties()
        {
            return const_cast<PropertySet&>(std::as_const(*this).derivedValidProperties());
        }

        bool derivedIsValid() const
        {
            return Set() <= derivedValidProperties();
        }

        inline static std::optional<type::PropertyDescriptor> M_descriptorStorage;
        std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    };
}