// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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

        const_iterator_t begin() const;
        const_iterator_t cbegin() const;

        const_iterator_t end() const;
        const_iterator_t cend() const;

        const underlying_map_t& data() const;

        const Descriptor<>* find(std::string_view name, bool assertNotNull = false) const;
        Descriptor<>* find(std::string_view name, bool assertNotNull = false);

        const Descriptor<>& get(std::string_view name) const;
        Descriptor<>& get(std::string_view name);

        size_t size() const;

        bool contains(std::string_view name) const;

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

        void erase(std::string_view name, bool assertContainedType = true);
        void erase(const Descriptor<>& descriptor, bool assertContainedType = true);

        void clear();

    private:

        underlying_map_t m_underlyingMap;
    };
}
