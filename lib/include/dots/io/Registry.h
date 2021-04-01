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

        template <typename Handler>
        void forEach(Handler&& handler)
        {
            constexpr bool is_type_handler = std::is_invocable_v<Handler, const type::Descriptor<>&>;
            static_assert(is_type_handler, "Handler has to be a valid type handler type");

            if constexpr (is_type_handler)
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

        std::shared_ptr<type::Descriptor<>> findType(const std::string_view& name, bool assertNotNull = false) const;
        std::shared_ptr<type::EnumDescriptor<>> findEnumType(const std::string_view& name, bool assertNotNull = false) const;
        std::shared_ptr<type::StructDescriptor<>> findStructType(const std::string_view& name, bool assertNotNull = false) const;

        const type::Descriptor<>& getType(const std::string_view& name) const;
        const type::EnumDescriptor<>& getEnumType(const std::string_view& name) const;
        const type::StructDescriptor<>& getStructType(const std::string_view& name) const;

        bool hasType(const std::string_view& name) const;

        std::shared_ptr<type::Descriptor<>> registerType(std::shared_ptr<type::Descriptor<>> descriptor, bool assertNewType = true);

        template <typename D, std::enable_if_t<std::is_base_of_v<type::Descriptor<>, D>, int> = 0>
        std::shared_ptr<D> registerType(D&& descriptor)
        {
            return std::static_pointer_cast<D>(registerType(std::make_shared<D>(std::forward<D>(descriptor))));
        }

        void deregisterType(const std::shared_ptr<type::Descriptor<>>& descriptor, bool assertRegisteredType = true);
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