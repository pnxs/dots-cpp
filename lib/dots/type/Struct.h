#pragma once
#include <string_view>
#include <array>
#include <functional>
#include <type_traits>
#include <dots/type/PropertyContainer.h>
#include <dots/type/StructDescriptor.h>

namespace dots::type
{
    struct Struct : PropertyContainer<Struct>
    {
        Struct(const StructDescriptor<>& descriptor);
        Struct(const Struct& other) = default;
        Struct(Struct&& other) noexcept = default;
        ~Struct() = default;

        Struct& operator = (const Struct& rhs) = default;
        Struct& operator = (Struct&& rhs) noexcept = default;

        const StructDescriptor<>& _descriptor() const;

		bool _usesDynamicMemory() const;
    	size_t _dynamicMemoryUsage() const;
		size_t _staticMemoryUsage() const;
		size_t _totalMemoryUsage() const;

		const PropertySet& _keyProperties() const;

		Struct& _assign(const Struct& other, const PropertySet& includedProperties = PropertySet::All);
		Struct& _copy(const Struct& other, const PropertySet& includedProperties = PropertySet::All);
		Struct& _merge(const Struct& other, const PropertySet& includedProperties = PropertySet::All);
		void _swap(Struct& other, const PropertySet& includedProperties = PropertySet::All);
		void _clear(const PropertySet& includedProperties = PropertySet::All);

		bool _equal(const Struct& rhs, const PropertySet& includedProperties = PropertySet::All) const;
    	bool _same(const Struct& rhs) const;
    	
		bool _less(const Struct& rhs, const PropertySet& includedProperties = PropertySet::All) const;
    	bool _lessEqual(const Struct& rhs, const PropertySet& includedProperties = PropertySet::All) const;
    	bool _greater(const Struct& rhs, const PropertySet& includedProperties = PropertySet::All) const;
    	bool _greaterEqual(const Struct& rhs, const PropertySet& includedProperties = PropertySet::All) const;

		PropertySet _diffProperties(const Struct& other, const PropertySet& includedProperties = PropertySet::All) const;

		void _publish(const PropertySet& includedProperties = PropertySet::All, bool remove = false) const;
		void _remove() const;

        template <bool AllowSubset = true>
        bool _hasProperties(const PropertySet& properties) const
        {
            if constexpr (AllowSubset)
            {
                return properties <= _validProperties();
            }
            else
            {
                return properties == _validProperties();
            }
        }

        template <bool AllowSubset = true>
        void _assertHasProperties(const PropertySet& expectedProperties) const
        {
            const PropertySet& actualProperties = _validProperties();

            if (!_hasProperties<AllowSubset>(expectedProperties))
            {
                auto to_property_list = [](const StructDescriptor<>& descriptor, const PropertySet& properties)
                {
                    std::string propertyList;

                    for (const PropertyDescriptor& propertyDescriptor : descriptor.propertyDescriptors(properties))
                    {
                        propertyList += propertyDescriptor.name();
                        propertyList += ", ";
                    }

                    if (!propertyList.empty())
                    {
                        propertyList.resize(propertyList.size() - 2);
                    }

                    return propertyList;
                };

                if constexpr (AllowSubset)
                {
                    throw std::logic_error{ _desc->name() + " instance is missing expected properties: " + to_property_list(*_desc, expectedProperties - actualProperties) };
                }
                else
                {
                    throw std::logic_error{ _desc->name() + " instance does not have expected exact properties: " + to_property_list(*_desc, expectedProperties) + ", but instead has " + to_property_list(*_desc, actualProperties) };
                }
            }
        }

        template <typename TDescriptor>
        bool _is(TDescriptor&& descriptor) const
        {
            static_assert(!std::is_rvalue_reference_v<TDescriptor>);
            static_assert(std::is_base_of_v<StructDescriptor<>, std::remove_pointer_t<std::decay_t<TDescriptor>>>);

            return &ToRef(std::forward<TDescriptor>(descriptor)) == _desc;
        }

        template <typename... Descriptors>
        bool _isAny(Descriptors&&... descriptors) const
        {
            static_assert(sizeof...(Descriptors) > 0);
            return (_is(std::forward<Descriptors>(descriptors)) || ...);
        }

        template <typename T>
        bool _is() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return _is(T::_Descriptor());
        }

    	template <typename... Ts>
        bool _isAny() const
        {
            static_assert(std::conjunction_v<std::is_base_of<Struct, Ts>...>);
            return _isAny(Ts::_Descriptor()...);
        }

        template <typename T>
        const T* _as() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return _is<T>() ? static_cast<const T*>(this) : nullptr;
        }

        template <typename T>
        T* _as()
        {
            return const_cast<T*>(std::as_const(*this)._as<T>());
        }

		template <typename T, bool Safe = false>
        const T& _to() const
        {
			static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");

			if constexpr (Safe)
			{
				if (!_is<T>())
				{
					throw std::logic_error{ std::string{ "type mismatch in safe Struct conversion: expected " } + _desc->name().data() + " but got " + T::_Descriptor().name() };
				}
			}

            return static_cast<const T&>(*this);
        }

        template <typename T, bool Safe = false>
        T& _to()
        {
            return const_cast<T&>(std::as_const(*this)._to<T, Safe>());
        }

    private:

    	friend struct PropertyContainer<Struct>;

    	const PropertyArea& derivedPropertyArea() const
		{
			return _desc->propertyArea(*this);
		}

    	PropertyArea& derivedPropertyArea()
		{
			return _desc->propertyArea(*this);
		}

    	const property_descriptor_container_t& derivedPropertyDescriptors() const
    	{
    		return _desc->propertyDescriptors();
    	}

        template <typename T>
        static decltype(auto) ToRef(T&& t)
        {
            if constexpr (std::is_pointer_v<T>)
            {
                return *t;
            }
            else
            {
                return t;
            }
        }

        const StructDescriptor<>* _desc;
    };
}