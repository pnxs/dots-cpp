#pragma once
#include <dots/type/StaticDescriptor.h>
#include <dots/type/Vector.h>

namespace dots::type
{
    template <>
    struct Descriptor<Vector<Typeless>> : Descriptor<Typeless>
    {
        Descriptor(std::string name, const std::shared_ptr<Descriptor<>>& valueDescriptor, size_t size, size_t alignment);
        Descriptor(const Descriptor& other) = default;
        Descriptor(Descriptor&& other) = default;
        ~Descriptor() = default;

        Descriptor& operator = (const Descriptor& rhs) = default;
        Descriptor& operator = (Descriptor&& rhs) = default;

        const std::shared_ptr<Descriptor<>>& valueDescriptorPtr() const;
        const Descriptor<Typeless>& valueDescriptor() const;

        virtual void fill(Vector<>& vector, size_t size) const = 0;

    private:

        std::shared_ptr<Descriptor<>> m_valueDescriptor;
    };

    template <typename T>
    struct Descriptor<Vector<T>> : StaticDescriptor<Vector<T>, Descriptor<Vector<Typeless>>>
    {
        static constexpr bool IsDynamic = is_dynamic_descriptor_v<Descriptor<T>>;

        Descriptor() :
            StaticDescriptor<Vector<T>, Descriptor<Vector<Typeless>>>("vector<" + valueDescriptor().name() + ">", valueDescriptorPtr(), sizeof(Vector<T>), alignof(Vector<T>))
        {
            /* do nothing */
        }
        Descriptor(const std::shared_ptr<Descriptor<>>& valueDescriptorOverride, bool checkSize = true) :
            StaticDescriptor<Vector<T>, Descriptor<Vector<Typeless>>>("vector<" + valueDescriptorOverride->name() + ">", valueDescriptorOverride, sizeof(Vector<T>), alignof(Vector<T>))
        {
            if (checkSize && (valueDescriptorOverride->size() != sizeof(T) || valueDescriptorOverride->alignment() != alignof(T)))
            {
                throw std::logic_error{ "attempt to create vector descriptor with incompatible value type" };
            }
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
                for (const T& value : lhs)
                {
                    dynMemUsage += valueDescriptor().dynamicMemoryUsage(value);
                }
            }

            return dynMemUsage;
        }

        void fill(Vector<>& typelessVector, size_t size) const override
        {
            auto& vector = static_cast<Vector<T>&>(typelessVector);
            vector.clear();

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

        static const std::shared_ptr<Descriptor<T>>& valueDescriptorPtr()
        {
            return Descriptor<T>::InstancePtr();
        }

        static const Descriptor<T>& valueDescriptor()
        {
            return Descriptor<T>::Instance();
        }
    };

    using VectorDescriptor = Descriptor<Vector<Typeless>>;

    [[deprecated("only available for backwards compatibility")]]
    inline const VectorDescriptor* toVectorDescriptor(const Descriptor<>* descriptor)
    {
        return descriptor->type() == Type::Vector ? static_cast<const VectorDescriptor*>(descriptor) : nullptr;
    }
}