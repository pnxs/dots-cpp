#pragma once
#include <map>
#include <memory>
#include <functional>
#include <type_traits>
#include <dots/type/DescriptorMap.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/EnumDescriptor.h>
#include <dots/type/StructDescriptor.h>

namespace dots::io
{
    struct Registry
    {
        using new_type_handler_t = std::function<void(const type::Descriptor<>&)>;

        Registry(new_type_handler_t newTypeHandler = nullptr, bool staticUserTypes = true);
        Registry(const Registry& other) = default;
        Registry(Registry&& other) noexcept = default;
        ~Registry() = default;

        Registry& operator = (const Registry& rhs) = default;
        Registry& operator = (Registry&& rhs) noexcept = default;

        template <typename TypeHandler>
        void forEach(TypeHandler&& handler)
        {
            constexpr bool IsTypeHandler = std::is_invocable_v<TypeHandler, const type::Descriptor<>&>;
            static_assert(IsTypeHandler, "Handler has to be a valid type handler");

            if constexpr (IsTypeHandler)
            {
                for (const auto& [name, descriptor] : type::static_descriptors())
                {
                    if (m_staticUserTypes || !IsUserType(*descriptor))
                    {
                        (void)name;
                        handler(*descriptor);
                    }
                }

                for (const auto& [name, descriptor] : m_types)
                {
                    (void)name;
                    handler(*descriptor);
                }
            }
        }

        template <typename... TDescriptors, typename TypeHandler, std::enable_if_t<sizeof...(TDescriptors) >= 1, int> = 0>
        void forEach(TypeHandler&& handler)
        {
            constexpr bool AreDescriptors = std::conjunction_v<std::is_base_of<type::Descriptor<>, TDescriptors>...>;
            constexpr bool IsTypeHandler = std::conjunction_v<std::is_invocable<TypeHandler, const TDescriptors&>...>;

            static_assert(AreDescriptors, "TDescriptor has to be a descriptor");
            static_assert(IsTypeHandler, "Handler has to be a valid type handler for all TDescriptors types");

            if constexpr (AreDescriptors && IsTypeHandler)
            {
                forEach([handler{ std::forward<TypeHandler>(handler) }](const type::Descriptor<>& descriptor)
                {
                    auto handle_type = [](auto& handler, const type::Descriptor<>& descriptor, const auto* wantedDescriptor)
                    {
                        using wanted_descriptor_t = std::decay_t<std::remove_pointer_t<decltype(wantedDescriptor)>>;
                        (void)wantedDescriptor;

                        if (wantedDescriptor = descriptor.as<wanted_descriptor_t>(); wantedDescriptor != nullptr)
                        {
                            std::invoke(handler, *wantedDescriptor);
                        }
                    };

                    (handle_type(handler, descriptor, static_cast<const TDescriptors*>(nullptr)), ...);
                });
            }
        }

        template <typename TypeHandler, typename TypeFilter>
        void forEach(TypeHandler&& handler, TypeFilter&& filter)
        {
            constexpr bool IsTypeFilter = std::is_invocable_r_v<bool, TypeFilter, const type::Descriptor<>&>;
            static_assert(IsTypeFilter, "Handler has to be a valid type filter");

            if constexpr (IsTypeFilter)
            {
                forEach([handler{ std::forward<TypeHandler>(handler) }, filter{ std::forward<TypeFilter>(filter) }](const type::Descriptor<>& descriptor)
                {
                    if (std::invoke(filter, descriptor))
                    {
                        std::invoke(handler, descriptor);
                    }
                });
            }
        }

        std::shared_ptr<const type::Descriptor<>> findType(const std::string_view& name, bool assertNotNull = false) const;
        std::shared_ptr<const type::EnumDescriptor<>> findEnumType(const std::string_view& name, bool assertNotNull = false) const;
        std::shared_ptr<const type::StructDescriptor<>> findStructType(const std::string_view& name, bool assertNotNull = false) const;

        std::shared_ptr<type::Descriptor<>> findType(const std::string_view& name, bool assertNotNull = false);
        std::shared_ptr<type::EnumDescriptor<>> findEnumType(const std::string_view& name, bool assertNotNull = false);
        std::shared_ptr<type::StructDescriptor<>> findStructType(const std::string_view& name, bool assertNotNull = false);

        const type::Descriptor<>& getType(const std::string_view& name) const;
        const type::EnumDescriptor<>& getEnumType(const std::string_view& name) const;
        const type::StructDescriptor<>& getStructType(const std::string_view& name) const;

        type::Descriptor<>& getType(const std::string_view& name);
        type::EnumDescriptor<>& getEnumType(const std::string_view& name);
        type::StructDescriptor<>& getStructType(const std::string_view& name);

        bool hasType(const std::string_view& name) const;

        std::shared_ptr<type::Descriptor<>> registerType(std::shared_ptr<type::Descriptor<>> descriptor, bool assertNewType = true);

        template <typename TDescriptor, typename... Args, std::enable_if_t<std::is_base_of_v<type::Descriptor<>, TDescriptor>, int> = 0>
        std::shared_ptr<TDescriptor> registerType(Args&&... args)
        {
            return std::static_pointer_cast<TDescriptor>(registerType(type::make_descriptor<TDescriptor>(std::forward<Args>(args)...)));
        }

        void deregisterType(std::shared_ptr<const type::Descriptor<>> descriptor, bool assertRegisteredType = true);
        void deregisterType(const type::Descriptor<>& descriptor, bool assertRegisteredType = true);
        void deregisterType(const std::string_view& name, bool assertRegisteredType = true);

        [[deprecated("only available for backwards compatibility")]]
        const type::Descriptor<>* findDescriptor(const std::string& name) const;

        [[deprecated("only available for backwards compatibility")]]
        const type::StructDescriptor<>* findStructDescriptor(const std::string& name) const;

        [[deprecated("only available for backwards compatibility")]]
        const std::map<std::string_view, std::shared_ptr<type::Descriptor<>>>& getTypes();

    private:

        static bool IsUserType(const type::Descriptor<>& descriptor);

        new_type_handler_t m_newTypeHandler;
        bool m_staticUserTypes;
        type::DescriptorMap m_types;
    };
}