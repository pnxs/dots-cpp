#include <dots/type/DescriptorMap.h>
#include <stdexcept>

namespace dots::type
{
    DescriptorMap::const_iterator_t DescriptorMap::begin() const
    {
        return m_underlyingMap.begin();
    }

    DescriptorMap::const_iterator_t DescriptorMap::cbegin() const
    {
        return m_underlyingMap.cbegin();
    }

    DescriptorMap::const_iterator_t DescriptorMap::end() const
    {
        return m_underlyingMap.end();
    }

    DescriptorMap::const_iterator_t DescriptorMap::cend() const
    {
        return m_underlyingMap.cend();
    }

    const DescriptorMap::underlying_map_t& DescriptorMap::data() const
    {
        return m_underlyingMap;
    }

    std::shared_ptr<const Descriptor<>> DescriptorMap::find(const std::string_view& name, bool assertNotNull/* = false*/) const
    {
        if (auto it = m_underlyingMap.find(name); it == m_underlyingMap.end())
        {
            if (assertNotNull)
            {
                throw std::logic_error{ std::string{ "no type with name: " } + name.data() };
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return it->second;
        }
    }

    std::shared_ptr<Descriptor<>> DescriptorMap::find(const std::string_view& name, bool assertNotNull)
    {
        return std::const_pointer_cast<Descriptor<>>(std::as_const(*this).find(name, assertNotNull));
    }

    const Descriptor<>& DescriptorMap::get(const std::string_view& name) const
    {
        return *find(name, true);
    }

    Descriptor<>& DescriptorMap::get(const std::string_view& name)
    {
        return const_cast<Descriptor<>&>(std::as_const(*this).get(name));
    }

    bool DescriptorMap::contains(const std::string_view& name) const
    {
        return find(name) == nullptr;
    }

    std::pair<std::shared_ptr<Descriptor<>>, bool> DescriptorMap::tryEmplace(std::shared_ptr<Descriptor<>> descriptor)
    {
        auto [it, emplaced] = m_underlyingMap.try_emplace(descriptor->name(), descriptor);
        return std::make_pair(it->second, emplaced);
    }

    std::shared_ptr<Descriptor<>> DescriptorMap::emplace(std::shared_ptr<Descriptor<>> descriptor)
    {
        auto [descriptor_, emplaced] = tryEmplace(std::move(descriptor));

        if (!emplaced)
        {
            throw std::logic_error{ "there already is a type with name: " + descriptor_->name() };
        }

        return descriptor_;
    }

    void DescriptorMap::erase(const std::string_view& name, bool assertContainedType)
    {
        if (auto it = m_underlyingMap.find(name); it == m_underlyingMap.end())
        {
            if (assertContainedType)
            {
                throw std::logic_error{ std::string{ "no type with name: " } + name.data() };
            }
        }
        else
        {
            m_underlyingMap.erase(it);
        }
    }

    void DescriptorMap::erase(std::shared_ptr<const Descriptor<>> descriptor, bool assertContainedType)
    {
        erase(descriptor->name(), assertContainedType);
    }

    void DescriptorMap::erase(const Descriptor<>& descriptor, bool assertContainedType)
    {
        erase(descriptor.name(), assertContainedType);
    }

    void DescriptorMap::clear()
    {
        m_underlyingMap.clear();
    }
}