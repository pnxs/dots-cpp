#pragma once
#include <memory>
#include <functional>
#include <type_traits>
#include <dots/type/DescriptorMap.h>
#include <dots/type/FundamentalTypes.h>
#include <dots/type/EnumDescriptor.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    struct Registry
    {
        using new_type_handler_t = std::function<void(const Descriptor<>&)>;

        Registry(new_type_handler_t newTypeHandler = nullptr, bool staticUserTypes = true);
        Registry(const Registry& other) = default;
        Registry(Registry&& other) noexcept = default;
        ~Registry() = default;

        Registry& operator = (const Registry& rhs) = default;
        Registry& operator = (Registry&& rhs) noexcept = default;

        template <typename TypeHandler>
        void forEach(TypeHandler&& handler)
        {
            constexpr bool IsTypeHandler = std::is_invocable_v<TypeHandler, const Descriptor<>&>;
            static_assert(IsTypeHandler, "Handler has to be a valid type handler");

            if constexpr (IsTypeHandler)
            {
                for (const auto& [name, descriptor] : static_descriptors())
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
            constexpr bool AreDescriptors = std::conjunction_v<std::is_base_of<Descriptor<>, TDescriptors>...>;
            constexpr bool IsTypeHandler = std::conjunction_v<std::is_invocable<TypeHandler, const TDescriptors&>...>;

            static_assert(AreDescriptors, "TDescriptor has to be a descriptor");
            static_assert(IsTypeHandler, "Handler has to be a valid type handler for all TDescriptors types");

            if constexpr (AreDescriptors && IsTypeHandler)
            {
                forEach([handler{ std::forward<TypeHandler>(handler) }](const Descriptor<>& descriptor)
                {
                    auto handle_type = [](auto& handler, const Descriptor<>& descriptor, const auto* wantedDescriptor)
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
            constexpr bool IsTypeFilter = std::is_invocable_r_v<bool, TypeFilter, const Descriptor<>&>;
            static_assert(IsTypeFilter, "Handler has to be a valid type filter");

            if constexpr (IsTypeFilter)
            {
                forEach([handler{ std::forward<TypeHandler>(handler) }, filter{ std::forward<TypeFilter>(filter) }](const Descriptor<>& descriptor)
                {
                    if (std::invoke(filter, descriptor))
                    {
                        std::invoke(handler, descriptor);
                    }
                });
            }
        }

        const Descriptor<>* findType(const std::string_view& name, bool assertNotNull = false) const;
        const EnumDescriptor<>* findEnumType(const std::string_view& name, bool assertNotNull = false) const;
        const StructDescriptor<>* findStructType(const std::string_view& name, bool assertNotNull = false) const;

        Descriptor<>* findType(const std::string_view& name, bool assertNotNull = false);
        EnumDescriptor<>* findEnumType(const std::string_view& name, bool assertNotNull = false);
        StructDescriptor<>* findStructType(const std::string_view& name, bool assertNotNull = false);

        const Descriptor<>& getType(const std::string_view& name) const;
        const EnumDescriptor<>& getEnumType(const std::string_view& name) const;
        const StructDescriptor<>& getStructType(const std::string_view& name) const;

        Descriptor<>& getType(const std::string_view& name);
        EnumDescriptor<>& getEnumType(const std::string_view& name);
        StructDescriptor<>& getStructType(const std::string_view& name);

        bool hasType(const std::string_view& name) const;

        Descriptor<>& registerType(Descriptor<>& descriptor, bool assertNewType = true);
        Descriptor<>& registerType(std::shared_ptr<Descriptor<>> descriptor, bool assertNewType = true);

        template <typename TDescriptor, typename... Args, std::enable_if_t<std::is_base_of_v<Descriptor<>, TDescriptor>, int> = 0>
        TDescriptor& registerType(Args&&... args)
        {
            return static_cast<TDescriptor&>(registerType(make_descriptor<TDescriptor>(std::forward<Args>(args)...)));
        }

        void deregisterType(const Descriptor<>& descriptor, bool assertRegisteredType = true);
        void deregisterType(const std::string_view& name, bool assertRegisteredType = true);

    private:

        static bool IsUserType(const Descriptor<>& descriptor);

        new_type_handler_t m_newTypeHandler;
        bool m_staticUserTypes;
        DescriptorMap m_types;
    };
}