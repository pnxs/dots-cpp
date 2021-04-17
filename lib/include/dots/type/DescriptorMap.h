#pragma once
#include <map>
#include <memory>
#include <dots/type/Descriptor.h>

namespace dots::type
{
    struct DescriptorMap
    {
        using underlying_map_t = std::map<std::string_view, std::shared_ptr<Descriptor<>>>;
        using const_iterator_t = underlying_map_t::const_iterator;

        DescriptorMap() = default;
        DescriptorMap(const DescriptorMap& other) = default;
        DescriptorMap(DescriptorMap&& other) noexcept = default;
        ~DescriptorMap() = default;

        DescriptorMap& operator = (const DescriptorMap& rhs) = default;
        DescriptorMap& operator = (DescriptorMap&& rhs) noexcept = default;

        const_iterator_t begin() const;
        const_iterator_t cbegin() const;

        const_iterator_t end() const;
        const_iterator_t cend() const;

        const underlying_map_t& data() const;

        const Descriptor<>* find(const std::string_view& name, bool assertNotNull = false) const;
        Descriptor<>* find(const std::string_view& name, bool assertNotNull = false);

        const Descriptor<>& get(const std::string_view& name) const;
        Descriptor<>& get(const std::string_view& name);

        bool contains(const std::string_view& name) const;

        std::pair<Descriptor<>*, bool> tryEmplace(Descriptor<>& descriptor);

        template <typename TDescriptor, typename... Args, std::enable_if_t<std::is_base_of_v<Descriptor<>, TDescriptor>, int> = 0>
        TDescriptor& tryEmplace(Args&&... args)
        {
            auto [descriptor, emplaced] = tryEmplace(std::make_shared<TDescriptor>(std::forward<Args>(args)...));
            return std::make_pair(std::static_pointer_cast<TDescriptor>(descriptor), emplaced);
        }

        Descriptor<>& emplace(Descriptor<>& descriptor);
        Descriptor<>& emplace(std::shared_ptr<Descriptor<>> descriptor);

        template <typename TDescriptor, typename... Args, std::enable_if_t<std::is_base_of_v<Descriptor<>, TDescriptor>, int> = 0>
        TDescriptor& emplace(Args&&... args)
        {
            return static_cast<TDescriptor&>(emplace(make_descriptor<TDescriptor>(std::forward<Args>(args)...)));
        }

        void erase(const std::string_view& name, bool assertContainedType = true);
        void erase(const Descriptor<>& descriptor, bool assertContainedType = true);

        void clear();

    private:

        underlying_map_t m_underlyingMap;
    };
}