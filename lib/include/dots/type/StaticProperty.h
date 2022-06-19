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
        template <typename... Args, std::enable_if_t<sizeof...(Args) >= 1 && !std::disjunction_v<is_property<Args>...>, int> = 0>
        StaticProperty(Args&&... args)
        {
            StaticProperty<T, Derived>::emplace(std::forward<Args>(args)...);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        StaticProperty(const Property<T, D>& other)
        {
            StaticProperty<T, Derived>::emplace(other);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        StaticProperty(Property<T, D>&& other)
        {
            Property<T, Derived>::emplace(std::move(other));
        }

        template <typename Arg, std::enable_if_t<!is_property_v<Arg>, int> = 0>
        Derived& operator = (Arg&& rhs)
        {
            Property<T, Derived>::emplace(std::forward<Arg>(rhs));
            return static_cast<Derived&>(*this);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        Derived& operator = (const Property<T, D>& other)
        {
            Property<T, Derived>::emplace(other);
            return static_cast<Derived&>(*this);
        }

        template <typename D, std::enable_if_t<!std::is_same_v<D, Derived>, int> = 0>
        Derived& operator = (Property<T, D>&& other)
        {
            Property<T, Derived>::emplace(std::move(other));
            return static_cast<Derived&>(*this);
        }

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

        static constexpr bool IsPartOf(PropertySet propertySet)
        {
            return Set() <= propertySet;
        }

        static const PropertyDescriptor& InitDescriptor() 
        {
            if (M_descriptorStorage == std::nullopt)
            {
                M_descriptorStorage.emplace(type::Descriptor<T>::Instance(), Name().data(), Tag(), IsKey(), Offset());
            }

            return *M_descriptorStorage; 
        }

        inline static const PropertyDescriptor& Descriptor = InitDescriptor();

    protected:

        StaticProperty() = default;

        StaticProperty(const StaticProperty& other) : Property<T, Derived>()
        {
            Property<T, Derived>::emplace(static_cast<const Derived&>(other));
        }

        StaticProperty(StaticProperty&& other)
        {
            Property<T, Derived>::emplace(static_cast<Derived&&>(other));
        }

        ~StaticProperty()
        {
            Property<T, Derived>::destroy();
        }

        StaticProperty& operator = (const StaticProperty& rhs)
        {
            Property<T, Derived>::emplace(static_cast<const Derived&>(rhs));
            return *this;
        }

        StaticProperty& operator = (StaticProperty&& rhs)
        {
            Property<T, Derived>::emplace(static_cast<Derived&&>(rhs));
            return *this;
        }

    private:

        friend struct Property<T, Derived>;

        PropertySet validProperties() const
        {
            return PropertyArea::GetArea(static_cast<const Derived&>(*this)).validProperties();
        }

        PropertySet& validProperties()
        {
            return PropertyArea::GetArea(static_cast<Derived&>(*this)).validProperties();
        }

        static const PropertyDescriptor& derivedDescriptor()
        {
            return Derived::Descriptor;
        }

        const T& derivedStorage() const
        {
            return reinterpret_cast<const T&>(m_storage);
        }

        bool derivedIsValid() const
        {
            return Set() <= validProperties();
        }

        void derivedSetValid()
        {
            validProperties() += derivedDescriptor().set();
        }

        void derivedSetInvalid()
        {
            validProperties() -= derivedDescriptor().set();
        }

        inline static std::optional<PropertyDescriptor> M_descriptorStorage;
        std::aligned_storage_t<sizeof(T), alignof(T)> m_storage;
    };
}
