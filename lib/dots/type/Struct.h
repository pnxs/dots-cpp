#pragma once
#include <string_view>
#include <array>
#include <functional>
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
    	
		bool _hasProperties(const PropertySet properties) const;

		void _publish(const PropertySet& includedProperties = PropertySet::All, bool remove = false) const;
		void _remove(const PropertySet& includedProperties = PropertySet::All) const;

        template <typename T>
        bool _is() const
        {
            static_assert(std::is_base_of_v<Struct, T>, "T has to be a sub-class of Struct");
            return &T::_Descriptor() == _desc;
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
			return _propArea;
		}

    	PropertyArea& derivedPropertyArea()
		{
			return _propArea;
		}

    	const property_descriptor_container_t& derivedPropertyDescriptors() const
    	{
    		return _desc->propertyDescriptors();
    	}

        const StructDescriptor<>* _desc;
    	PropertyArea _propArea;
    };
}