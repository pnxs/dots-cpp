#pragma once
#include <string_view>
#include <array>
#include <functional>
#include <dots/type/NewPropertyContainer.h>
#include <dots/type/NewStructDescriptor.h>

namespace dots::type
{
    struct NewStruct : NewPropertyContainer<NewStruct>
    {
        NewStruct(const NewStructDescriptor<>& descriptor);
        NewStruct(const NewStruct& other) = default;
        NewStruct(NewStruct&& other) noexcept = default;
        ~NewStruct() = default;

        NewStruct& operator = (const NewStruct& rhs) = default;
        NewStruct& operator = (NewStruct&& rhs) noexcept = default;

        const NewStructDescriptor<>& _descriptor() const;

		bool _usesDynamicMemory() const;
    	size_t _dynamicMemoryUsage() const;
		size_t _staticMemoryUsage() const;
		size_t _totalMemoryUsage() const;

		const NewPropertySet& _keyProperties() const;

		NewStruct& _assign(const NewStruct& other, const NewPropertySet& includedProperties = NewPropertySet::All);
		NewStruct& _copy(const NewStruct& other, const NewPropertySet& includedProperties = NewPropertySet::All);
		NewStruct& _merge(const NewStruct& other, const NewPropertySet& includedProperties = NewPropertySet::All);
		void _swap(NewStruct& other, const NewPropertySet& includedProperties = NewPropertySet::All);
		void _clear(const NewPropertySet& includedProperties = NewPropertySet::All);

		bool _equal(const NewStruct& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const;
    	bool _same(const NewStruct& rhs) const;
    	
		bool _less(const NewStruct& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const;
    	bool _lessEqual(const NewStruct& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const;
    	bool _greater(const NewStruct& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const;
    	bool _greaterEqual(const NewStruct& rhs, const NewPropertySet& includedProperties = NewPropertySet::All) const;

		NewPropertySet _diffProperties(const NewStruct& other, const NewPropertySet& includedProperties = NewPropertySet::All) const;
    	
		bool _hasProperties(const NewPropertySet properties) const;

		/*void _publish(const NewPropertySet& what = NewPropertySet::All, bool remove = false) const;
		void _remove(const NewPropertySet& what = NewPropertySet::All) const;*/

        template <typename T>
        bool _is() const
        {
            static_assert(std::is_base_of_v<NewStruct, T>, "T has to be a sub-class of NewStruct");
            return &T::_Descriptor() == _desc;
        }

        template <typename T>
        const T* _as() const
        {
            static_assert(std::is_base_of_v<NewStruct, T>, "T has to be a sub-class of NewStruct");
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
			static_assert(std::is_base_of_v<NewStruct, T>, "T has to be a sub-class of NewStruct");

			if constexpr (Safe)
			{
				if (!_is<T>())
				{
					throw std::logic_error{ std::string{ "type mismatch in safe NewStruct conversion: expected " } + _desc->name().data() + " but got " + T::_Descriptor().name() };
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

    	friend struct NewPropertyContainer<NewStruct>;

    	const NewPropertyArea& derivedPropertyArea() const
		{
			return _propArea;
		}

    	NewPropertyArea& derivedPropertyArea()
		{
			return _propArea;
		}

    	const new_property_descriptor_container_t& derivedPropertyDescriptors() const
    	{
    		return _desc->propertyDescriptors();
    	}

        const NewStructDescriptor<>* _desc;
    	NewPropertyArea _propArea;
    };
}