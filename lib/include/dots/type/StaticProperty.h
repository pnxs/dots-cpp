#pragma once
#include <type_traits>
#include <dots/type/Property.h>

namespace dots::type
{
    template <typename T, typename Derived>
    struct StaticProperty : Property<T, Derived>
    {
        using Property<T, Derived>::operator=;

        static std::string_view Name()
        {
            return Derived::Descriptor.name();
        }

        static uint32_t Tag()
        {
            return Derived::Descriptor.tag();
        }

        static bool IsKey()
        {
            return Derived::Descriptor.isKey();
        }

        static PropertySet Set()
        {
            return Derived::Descriptor.set();
        }

        static bool IsPartOf(const PropertySet& propertySet)
        {
            return Set() <= propertySet;
        }

    protected:

        StaticProperty()
        {
            /* do nothing */
        }

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
            return m_storage;
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

        union
        {
            T m_storage;
            std::aligned_storage_t<sizeof(T), alignof(T)> m_data;
        };
    };
}