#pragma once
#include <dots/type/StaticDescriptor.h>
#include <dots/type/Vector.h>

namespace dots::type
{
    template <>
    struct Descriptor<Vector<>> : StaticDescriptor
    {
        Descriptor(key_t key, std::string name, Descriptor<>& valueDescriptor, size_t size, size_t alignment);
        Descriptor(const Descriptor& other) = delete;
        Descriptor(Descriptor&& other) = delete;
        ~Descriptor() override = default;

        Descriptor& operator = (const Descriptor& rhs) = delete;
        Descriptor& operator = (Descriptor&& rhs) = delete;

        const Descriptor<Typeless>& valueDescriptor() const;
        Descriptor<Typeless>& valueDescriptor();

        virtual void resize(Vector<>& vector, size_t size) const = 0;
        virtual void fill(Vector<>& vector, size_t size) const = 0;

    private:

        std::shared_ptr<Descriptor<>> m_valueDescriptor;
    };

    template <typename T>
    struct Descriptor<Vector<T>> : Descriptor<Vector<>>
    {
        static constexpr bool IsDynamic = is_dynamic_descriptor_v<Descriptor<T>>;

        template <bool IsDynamic = !IsDynamic, std::enable_if_t<IsDynamic, int> = 0>
        Descriptor(key_t key) :
            Descriptor<Vector<>>(key, "vector<" + Descriptor<T>::Instance().name() + ">", Descriptor<T>::Instance(), sizeof(Vector<T>), alignof(Vector<T>))
        {
            /* do nothing */
        }

        template <bool IsDynamic = IsDynamic, std::enable_if_t<IsDynamic, int> = 0>
        Descriptor(key_t key, Descriptor<T>& valueDescriptorOverride, bool checkSize = true) :
            Descriptor<Vector<>>(key, "vector<" + valueDescriptorOverride.name() + ">", valueDescriptorOverride, sizeof(Vector<T>), alignof(Vector<T>))
        {
            if (checkSize && (valueDescriptorOverride.size() != sizeof(T) || valueDescriptorOverride.alignment() != alignof(T)))
            {
                throw std::logic_error{ "attempt to create vector descriptor with incompatible value type" };
            }
        }

        using StaticDescriptor::construct;
        using StaticDescriptor::constructInPlace;
        using StaticDescriptor::destruct;
        using StaticDescriptor::assign;
        using StaticDescriptor::swap;
        using StaticDescriptor::equal;
        using StaticDescriptor::less;
        using StaticDescriptor::lessEqual;
        using StaticDescriptor::greater;
        using StaticDescriptor::greaterEqual;
        using StaticDescriptor::dynamicMemoryUsage;

        Typeless& construct(Typeless& value) const override
        {
            return reinterpret_cast<Typeless&>(construct(reinterpret_cast<Vector<T>&>(value)));
        }

        Typeless& construct(Typeless& value, const Typeless& other) const override
        {
            return reinterpret_cast<Typeless&>(construct(reinterpret_cast<Vector<T>&>(value), reinterpret_cast<const Vector<T>&>(other)));
        }

        Typeless& construct(Typeless& value, Typeless&& other) const override
        {
            return reinterpret_cast<Typeless&>(construct(reinterpret_cast<Vector<T>&>(value), reinterpret_cast<Vector<T>&&>(other)));
        }

        Typeless& constructInPlace(Typeless& value) const override
        {
            return construct(value);
        }

        Typeless& constructInPlace(Typeless& value, const Typeless& other) const override
        {
            return construct(value, other);
        }

        Typeless& constructInPlace(Typeless& value, Typeless&& other) const override
        {
            return construct(value, std::move(other));
        }

        void destruct(Typeless& value) const override
        {
            destruct(reinterpret_cast<Vector<T>&>(value));
        }

        Typeless& assign(Typeless& lhs, const Typeless& rhs) const override
        {
            return reinterpret_cast<Typeless&>(assign(reinterpret_cast<Vector<T>&>(lhs), reinterpret_cast<const Vector<T>&>(rhs)));
        }

        Typeless& assign(Typeless& lhs, Typeless&& rhs) const override
        {
            return reinterpret_cast<Typeless&>(assign(reinterpret_cast<Vector<T>&>(lhs), reinterpret_cast<Vector<T>&&>(rhs)));
        }

        void swap(Typeless& value, Typeless& other) const override
        {
            swap(reinterpret_cast<Vector<T>&>(value), reinterpret_cast<Vector<T>&>(other));
        }

        bool equal(const Typeless& lhs, const Typeless& rhs) const override
        {
            return equal(reinterpret_cast<const Vector<T>&>(lhs), reinterpret_cast<const Vector<T>&>(rhs));
        }

        bool less(const Typeless& lhs, const Typeless& rhs) const override
        {
            return less(reinterpret_cast<const Vector<T>&>(lhs), reinterpret_cast<const Vector<T>&>(rhs));
        }

        bool usesDynamicMemory() const override
        {
            return true;
        }

        size_t dynamicMemoryUsage(const Typeless& lhs) const override
        {
            return dynamicMemoryUsage(lhs.to<Vector<T>>());
        }

        size_t dynamicMemoryUsage(const Vector<T>& lhs) const
        {
            size_t size = lhs.size();
            size_t dynMemUsage = size * valueDescriptor().size();

            if (valueDescriptor().usesDynamicMemory())
            {
                for (const auto& value : lhs)
                {
                    dynMemUsage += valueDescriptor().dynamicMemoryUsage(value);
                }
            }

            return dynMemUsage;
        }

        void resize(Vector<>& typelessVector, size_t size) const override
        {
            auto& vector = static_cast<Vector<T>&>(typelessVector);

            if constexpr (std::is_default_constructible_v<T>)
            {
                vector.resize(size);
            }
            else
            {
                const auto& valueDescriptor = Descriptor::valueDescriptor();
                std::aligned_storage_t<sizeof(T), alignof(T)> rawStorage;
                T& storage = reinterpret_cast<T&>(rawStorage);

                for (size_t i = 0; i < size; ++i)
                {
                    valueDescriptor.construct(storage);
                    vector.emplace_back(std::move(storage));
                    valueDescriptor.destruct(storage);
                }
            }
        }

        void fill(Vector<>& typelessVector, size_t size) const override
        {
            auto& vector = static_cast<Vector<T>&>(typelessVector);
            vector.clear();
            resize(typelessVector, size);
        }

        const Descriptor<T>& valueDescriptor() const
        {
            return static_cast<const Descriptor<T>&>(Descriptor<Vector<>>::valueDescriptor());
        }

        Descriptor<T>& valueDescriptor()
        {
            return const_cast<Descriptor<T>&>(std::as_const(*this).valueDescriptor());
        }

        bool isFundamentalType() const override
        {
            return valueDescriptor().isFundamentalType();
        }

        static auto& Instance()
        {
            return InitInstance<Vector<T>>();
        }
    };

    template <typename TDescriptor>
    struct type_category<TDescriptor, std::enable_if_t<std::is_same_v<Descriptor<Vector<>>, TDescriptor>>> : std::integral_constant<Type, Type::Vector> {};

    using VectorDescriptor = Descriptor<Vector<Typeless>>;
}
