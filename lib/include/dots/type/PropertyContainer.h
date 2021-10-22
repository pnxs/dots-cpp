#pragma once
#include <string_view>
#include <type_traits>
#include <algorithm>
#include <dots/type/PropertyArea.h>
#include <dots/type/PropertyPairIterator.h>

namespace dots::type
{
    template <typename Derived>
    struct PropertyContainer
    {
        PropertyContainer() = default;
        PropertyContainer(const PropertyContainer& other) = default;
        PropertyContainer(PropertyContainer&& other) = default;
        ~PropertyContainer() = default;

        PropertyContainer& operator = (const PropertyContainer& rhs) = default;
        PropertyContainer& operator = (PropertyContainer&& rhs) noexcept = default;

        const_property_iterator operator [] (PropertySet::index_t tag) const
        {
            return _find(tag);
        }

        property_iterator operator [] (PropertySet::index_t tag)
        {
            return _find(tag);
        }

        const_property_iterator operator [] (std::string_view name) const
        {
            return _find(name);
        }

        property_iterator operator [] (std::string_view name)
        {
            return _find(name);
        }

        constexpr operator const PropertyArea&() const
        {
            return _propertyArea();
        }

        constexpr operator PropertyArea&()
        {
            return _propertyArea();
        }

        constexpr const PropertyArea& _propertyArea() const
        {
            return static_cast<const Derived&>(*this).derivedPropertyArea();
        }

        constexpr PropertyArea& _propertyArea()
        {
            return static_cast<Derived&>(*this).derivedPropertyArea();
        }

        constexpr const PropertySet& _validProperties() const
        {
            return _propertyArea().validProperties();
        }

        constexpr const property_descriptor_container_t& _propertyDescriptors() const
        {
            return static_cast<const Derived&>(*this).derivedPropertyDescriptors();
        }

        constexpr const std::vector<PropertyPath>& _propertyPaths() const
        {
            return static_cast<const Derived&>(*this).derivedPropertyPaths();
        }

        property_iterator _begin(const PropertySet& includedProperties = PropertySet::All)
        {
            return property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
        }

        const_property_iterator _begin(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().begin(), includedProperties };
        }

        property_iterator _end(const PropertySet& includedProperties = PropertySet::All)
        {
            return property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
        }

        const_property_iterator _end(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().end(), includedProperties };
        }

        reverse_property_iterator _rbegin(const PropertySet& includedProperties = PropertySet::All)
        {
            return reverse_property_iterator{ _propertyArea(),_propertyDescriptors(),  _propertyDescriptors().rbegin(), includedProperties };
        }

        const_reverse_property_iterator _rbegin(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_reverse_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rbegin(), includedProperties };
        }

        reverse_property_iterator _rend(const PropertySet& includedProperties = PropertySet::All)
        {
            return reverse_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
        }

        const_reverse_property_iterator _rend(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_reverse_property_iterator{ _propertyArea(), _propertyDescriptors(), _propertyDescriptors().rend(), includedProperties };
        }

        property_range _propertyRange(const PropertySet& includedProperties = PropertySet::All)
        {
            return property_range{ _begin(includedProperties), _end(includedProperties) };
        }

        const_property_range _propertyRange(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_property_range{ _begin(includedProperties), _end(includedProperties) };
        }

        reverse_property_range _propertyRangeReversed(const PropertySet& includedProperties = PropertySet::All)
        {
            return reverse_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
        }

        const_reverse_property_range _propertyRangeReversed(const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_reverse_property_range{ _rbegin(includedProperties), _rend(includedProperties) };
        }

        property_pair_range _propertyRange(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return property_pair_range{ property_pair_iterator{ _begin(includedProperties), rhs._begin(includedProperties) }, property_pair_iterator{ _end(includedProperties), rhs._end(includedProperties) } };
        }

        property_pair_range_const _propertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return property_pair_range_const{ property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
        }

        const_property_pair_range_const _propertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_property_pair_range_const{ const_property_pair_iterator_const{ _begin(includedProperties), rhs._begin(includedProperties) }, const_property_pair_iterator_const{ _end(includedProperties), rhs._end(includedProperties) } };
        }

        reverse_property_pair_range _propertyRangeReversed(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return reverse_property_pair_range{ reverse_property_pair_iterator{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_property_pair_iterator{ _rend(includedProperties), rhs._rend(includedProperties) } };
        }

        reverse_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return reverse_property_pair_range_const{ reverse_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, reverse_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
        }

        const_reverse_property_pair_range_const _propertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
        {
            return const_reverse_property_pair_range_const{ const_reverse_property_pair_iterator_const{ _rbegin(includedProperties), rhs._rbegin(includedProperties) }, const_reverse_property_pair_iterator_const{ _rend(includedProperties), rhs._rend(includedProperties) } };
        }

        property_range _validPropertyRange(const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRange(_validProperties() ^ includedProperties);
        }

        const_property_range _validPropertyRange(const PropertySet& includedProperties = PropertySet::All) const
        {
            return _propertyRange(_validProperties() ^ includedProperties);
        }

        reverse_property_range _validPropertyRangeReversed(const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRangeReversed(_validProperties() ^ includedProperties);
        }

        const_reverse_property_range _validPropertyRangeReversed(const PropertySet& includedProperties = PropertySet::All) const
        {
            return _propertyRangeReversed(_validProperties() ^ includedProperties);
        }

        property_pair_range _validPropertyRange(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        property_pair_range_const _validPropertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        const_property_pair_range_const _validPropertyRange(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
        {
            return _propertyRange(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        reverse_property_pair_range _validPropertyRangeReversed(Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        reverse_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All)
        {
            return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        const_reverse_property_pair_range_const _validPropertyRangeReversed(const Derived& rhs, const PropertySet& includedProperties = PropertySet::All) const
        {
            return _propertyRangeReversed(rhs, _validProperties() ^ rhs._validProperties() ^ includedProperties);
        }

        template <typename Callable, std::enable_if_t<std::is_invocable_v<Callable, const ProxyProperty<>&>, int> = 0>
        const_property_iterator _find(Callable&& callable) const
        {
            return std::find_if(_begin(), _end(), std::forward<Callable>(callable));
        }

        template <typename Callable, std::enable_if_t<std::is_invocable_v<Callable, const ProxyProperty<>&>, int> = 0>
        property_iterator _find(Callable&& callable)
        {
            return std::find_if(_begin(), _end(), std::forward<Callable>(callable));
        }

        const_property_iterator _find(PropertySet::index_t tag) const
        {
            return _find([&](const ProxyProperty<>& property)
            {
                return property.descriptor().tag() == tag;
            });
        }

        property_iterator _find(PropertySet::index_t tag)
        {
            return _find([&](const ProxyProperty<>& property)
            {
                return property.descriptor().tag() == tag;
            });
        }

        const_property_iterator _find(std::string_view name) const
        {
            return _find([&](const ProxyProperty<>& property)
            {
                return property.descriptor().name() == name;
            });
        }

        property_iterator _find(std::string_view name)
        {
            return _find([&](const ProxyProperty<>& property)
            {
                return property.descriptor().name() == name;
            });
        }

        const PropertyPath& _path(std::string_view propertyPath) const
        {
            std::vector<std::string_view> propertyNames;

            for (std::string_view propertyPath_ = propertyPath;;)
            {
                std::string_view::size_type delimiterPos = propertyPath_.find_first_of('.');
                propertyNames.emplace_back(propertyPath_.substr(0, delimiterPos));

                if (delimiterPos == std::string_view::npos)
                {
                    break;
                }
                else
                {
                    if (++delimiterPos > propertyPath_.size())
                    {
                        throw std::runtime_error{ "invalid composed property name '" + std::string{ propertyPath } + "'" };
                    }

                    propertyPath_ = propertyPath_.substr(delimiterPos);
                }
            }

            for (const PropertyPath& path : _propertyPaths())
            {
                auto equal_names = [](const PropertyDescriptor& propertyDescriptor, std::string_view propertyName)
                {
                    return propertyDescriptor.name() == propertyName;
                };

                if (std::equal(path.elements().begin(), path.elements().end(), propertyNames.begin(), propertyNames.end(), equal_names))
                {
                    return path;
                }
            }

            throw std::runtime_error{ "unknown composed property name '" + std::string{ propertyPath } + "'" };

        }

        template <typename T = Typeless>
        ProxyProperty<T> _get(const PropertyPath& propertyPath)
        {
            return ProxyProperty<T>{ _propertyArea(), propertyPath };
        }

        template <typename T = Typeless>
        ProxyProperty<T> _get(std::string_view propertyPath)
        {
            return _get<T>(_path(propertyPath));
        }
    };

    template <typename Derived>
    property_iterator begin(PropertyContainer<Derived>& container)
    {
        return container._begin();
    }

    template <typename Derived>
    const_property_iterator begin(const PropertyContainer<Derived>& container)
    {
        return container._begin();
    }

    template <typename Derived>
    property_iterator end(PropertyContainer<Derived>& container)
    {
        return container._end();
    }

    template <typename Derived>
    const_property_iterator end(const PropertyContainer<Derived>& container)
    {
        return container._end();
    }

    template <typename Derived>
    reverse_property_iterator rbegin(PropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

    template <typename Derived>
    const_reverse_property_iterator rbegin(const PropertyContainer<Derived>& container)
    {
        return container._rbegin();
    }

    template <typename Derived>
    reverse_property_iterator rend(PropertyContainer<Derived>& container)
    {
        return container._rend();
    }

    template <typename Derived>
    const_reverse_property_iterator rend(const PropertyContainer<Derived>& container)
    {
        return container._rend();
    }
}