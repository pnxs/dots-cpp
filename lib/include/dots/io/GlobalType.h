#pragma once
#include <functional>
#include <dots/type/StructDescriptor.h>
#include <dots/type/DescriptorMap.h>

namespace dots::io
{
    namespace details
    {
        template <typename Tag, typename T = void, typename = void>
        struct global_type;

        template <typename Tag>
        struct global_type<Tag, void>
        {
            inline static type::DescriptorMap Descriptors;
        };

        template <typename Tag, typename T>
        struct global_type<Tag, T> : global_type<Tag, void>
        {
            inline static const type::StructDescriptor<T>& Descriptor = static_cast<const type::StructDescriptor<T>&>(*global_type<Tag, void>::Descriptors.emplace(type::Descriptor<T>::InstancePtr()));
        };

        template <typename Tag, typename T>
        const type::StructDescriptor<T>& register_global_type()
        {
            return global_type<Tag, T>::Descriptor;
        }

        struct global_publish_tag{};
        struct global_subscribe_tag{};
    }

    template <typename T>
    const type::StructDescriptor<T>& register_global_publish_type()
    {
        return details::register_global_type<details::global_publish_tag, T>();
    }

    inline const type::DescriptorMap& global_publish_types()
    {
        return details::global_type<details::global_publish_tag, void>::Descriptors;
    }

    template <typename T>
    const type::StructDescriptor<T>& register_global_subscribe_type()
    {
        return details::register_global_type<details::global_subscribe_tag, T>();
    }

    inline const type::DescriptorMap& global_subscribe_types()
    {
        return details::global_type<details::global_subscribe_tag, void>::Descriptors;
    }
}