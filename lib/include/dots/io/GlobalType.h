#pragma once
#include <functional>
#include <dots/type/StructDescriptor.h>

namespace dots::io
{
    using global_descriptors_t = std::deque<std::reference_wrapper<const type::StructDescriptor<>>>;

    namespace details
    {
        template <typename Tag, typename T = void, typename = void>
        struct global_type;

        template <typename Tag>
        struct global_type<Tag, void>
        {
            static const global_descriptors_t& Descriptors()
            {
                return DescriptorsMutable();
            }

        protected:

            static global_descriptors_t& DescriptorsMutable()
            {
                static global_descriptors_t Descriptors_;
                return Descriptors_;
            }
        };

        template <typename Tag, typename T>
        struct global_type<Tag, T> : global_type<Tag, void>
        {
            inline static const type::StructDescriptor<T>& Descriptor = static_cast<const type::StructDescriptor<T>&>(global_type<Tag, void>::DescriptorsMutable().emplace_back(type::Descriptor<T>::Instance()).get());
        };

        template <typename Tag, typename T>
        const type::StructDescriptor<T>& register_global_type()
        {
            return global_type<Tag, T>::Descriptor;
        }

        template <typename Tag>
        const global_descriptors_t& global_types()
        {
            return global_type<Tag, void>::Descriptors();
        }

        struct global_publish_tag{};
        struct global_subscribe_tag{};
    }

    template <typename T>
    const type::StructDescriptor<T>& register_global_publish_type()
    {
        return details::register_global_type<details::global_publish_tag, T>();
    }

    inline const global_descriptors_t& global_publish_types()
    {
        return details::global_type<details::global_publish_tag, void>::Descriptors();
    }

    template <typename T>
    const type::StructDescriptor<T>& register_global_subscribe_type()
    {
        return details::register_global_type<details::global_subscribe_tag, T>();
    }

    inline const global_descriptors_t& global_subscribe_types()
    {
        return details::global_type<details::global_subscribe_tag, void>::Descriptors();
    }
}