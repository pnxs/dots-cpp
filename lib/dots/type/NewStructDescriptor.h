#pragma once
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewStaticDescriptor.h>
#include <dots/type/NewProperty.h>

namespace dots::type
{
	struct NewStruct;

	template <typename T = NewTypeless, typename = void>
	struct NewStructDescriptor;

	template <>
	struct NewStructDescriptor<NewTypeless> : NewDescriptor<NewTypeless>
	{
		static const uint8_t Uncached      = 0b0000'0000;
		static const uint8_t Cached        = 0b0000'0001;
		static const uint8_t Internal      = 0b0000'0010;
		static const uint8_t Persistent    = 0b0000'0100;
		static const uint8_t Cleanup       = 0b0000'1000;
		static const uint8_t Local         = 0b0001'0000;
		static const uint8_t SubstructOnly = 0b0010'0000;
		
		NewStructDescriptor(std::string name, uint8_t flags, const new_property_descriptor_container_t& propertyDescriptors, size_t size, size_t alignment);
		NewStructDescriptor(const NewStructDescriptor& other) = default;
		NewStructDescriptor(NewStructDescriptor&& other) = default;
		~NewStructDescriptor() = default;

		NewStructDescriptor& operator = (const NewStructDescriptor& rhs) = default;
		NewStructDescriptor& operator = (NewStructDescriptor&& rhs) = default;

		NewTypeless& construct(NewTypeless& value) const override;
		NewStruct& construct(NewStruct& instance) const;
		NewTypeless& construct(NewTypeless& value, const NewTypeless& other) const override;
		NewStruct& construct(NewStruct& instance, const NewStruct& other) const;
		NewTypeless& construct(NewTypeless& value, NewTypeless&& other) const override;
		NewStruct& construct(NewStruct& instance, NewStruct&& other) const;
		
		void destruct(NewTypeless& value) const override;
		NewStruct& destruct(NewStruct& instance) const;
		
		NewTypeless& assign(NewTypeless& lhs, const NewTypeless& rhs) const override;
		NewTypeless& assign(NewTypeless& lhs, NewTypeless&& rhs) const override;
		void swap(NewTypeless& value, NewTypeless& other) const override;
		
		bool equal(const NewTypeless& lhs, const NewTypeless& rhs) const override;
		bool less(const NewTypeless& lhs, const NewTypeless& rhs) const override;
		bool lessEqual(const NewTypeless& lhs, const NewTypeless& rhs) const override;
		bool greater(const NewTypeless& lhs, const NewTypeless& rhs) const override;
		bool greaterEqual(const NewTypeless& lhs, const NewTypeless& rhs) const override;

		bool usesDynamicMemory() const override;
		size_t dynamicMemoryUsage(const NewTypeless& instance) const override;
		size_t dynamicMemoryUsage(const NewStruct& instance) const;

		virtual NewStruct& assign(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const;
		virtual NewStruct& copy(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const;
		virtual NewStruct& merge(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const;
		virtual void swap(NewStruct& instance, NewStruct& other, const NewPropertySet& includedProperties) const;
		virtual void clear(NewStruct& instance, const NewPropertySet& includedProperties) const;

		virtual bool equal(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const;
		virtual bool same(const NewStruct& lhs, const NewStruct& rhs) const;
		
		virtual bool less(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const;
		virtual bool lessEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const;
		virtual bool greater(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const;
		virtual bool greaterEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const;

		virtual NewPropertySet diffProperties(const NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const;

		const NewPropertyArea& propertyArea(const NewStruct& instance) const;
		NewPropertyArea& propertyArea(NewStruct& instance) const;

		uint8_t flags() const;
		bool cached() const;
		bool cleanup() const;
		bool local() const;
		bool persistent() const;
		bool internal() const;
		bool substructOnly() const;

		const new_property_descriptor_container_t& propertyDescriptors() const;
		
		const NewPropertySet& properties() const;
		const NewPropertySet& keyProperties() const;

	private:

		uint8_t m_flags;
		new_property_descriptor_container_t m_propertyDescriptors;
		NewPropertySet m_properties;
		NewPropertySet m_keyProperties;
		NewPropertySet m_dynamicMemoryProperties;
	};

	template <typename T>
	struct NewStructDescriptor<T> : NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>
	{
		static_assert(std::is_base_of_v<NewStruct, T>);
		
		NewStructDescriptor(std::string name, uint8_t flags, const new_property_descriptor_container_t& propertyDescriptor) :
			NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>(std::move(name), flags, propertyDescriptor, sizeof(T), alignof(T))
		{
			/* do nothing */
		}
		NewStructDescriptor(const NewStructDescriptor& other) = default;
		NewStructDescriptor(NewStructDescriptor&& other) = default;
		~NewStructDescriptor() = default;

		NewStructDescriptor& operator = (const NewStructDescriptor& rhs) = default;
		NewStructDescriptor& operator = (NewStructDescriptor&& rhs) = default;

		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::assign;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::swap;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::equal;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::less;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::lessEqual;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::greater;
		using NewStaticDescriptor<T, NewStructDescriptor<NewTypeless>>::greaterEqual;

		NewStruct& assign(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const override
		{
			return assign(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& assign(T& instance, const T& other, const NewPropertySet& includedProperties) const
		{
			return instance._assign(other, includedProperties);
		}

		NewStruct& copy(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const override
		{
			return copy(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& copy(T& instance, const T& other, const NewPropertySet& includedProperties) const
		{
			return instance._copy(other, includedProperties);
		}

		NewStruct& merge(NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const override
		{
			return merge(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& merge(T& instance, const T& other, const NewPropertySet& includedProperties) const
		{
			return instance._merge(other, includedProperties);
		}
		
		void swap(NewStruct& instance, NewStruct& other, const NewPropertySet& includedProperties) const override
		{
			swap(static_cast<T&>(instance), static_cast<T&>(other), includedProperties);
		}

		void swap(T& instance, T& other, const NewPropertySet& includedProperties) const
		{
			instance._swap(other, includedProperties);
		}

		void clear(NewStruct& instance, const NewPropertySet& includedProperties) const override
		{
			clear(static_cast<T&>(instance), includedProperties);
		}

		void clear(T& instance, const NewPropertySet& includedProperties) const
		{
			instance._clear(includedProperties);
		}

		bool equal(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const override
		{
			return equal(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool equal(const T& lhs, const T& rhs, const NewPropertySet& includedProperties) const
		{
			return lhs._equal(rhs, includedProperties);
		}

		bool same(const NewStruct& lhs, const NewStruct& rhs) const override
		{
			return same(static_cast<const T&>(lhs), static_cast<const T&>(rhs));
		}

		bool same(const T& lhs, const T& rhs) const
		{
			return lhs._same(rhs);
		}

		bool less(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const override
		{
			return less(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool less(const T& lhs, const T& rhs, const NewPropertySet& includedProperties) const
		{
			return lhs._less(rhs, includedProperties);
		}

		bool lessEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const override
		{
			return lessEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool lessEqual(const T& lhs, const T& rhs, const NewPropertySet& includedProperties) const
		{
			return lhs._lessEqual(rhs, includedProperties);
		}

		bool greater(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const override
		{
			return greater(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool greater(const T& lhs, const T& rhs, const NewPropertySet& includedProperties) const
		{
			return lhs._greater(rhs, includedProperties);
		}

		bool greaterEqual(const NewStruct& lhs, const NewStruct& rhs, const NewPropertySet& includedProperties) const override
		{
			return greaterEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool greaterEqual(const T& lhs, const T& rhs, const NewPropertySet& includedProperties) const
		{
			return lhs._greaterEqual(rhs, includedProperties);
		}

		NewPropertySet diffProperties(const NewStruct& instance, const NewStruct& other, const NewPropertySet& includedProperties) const override
		{
			return diffProperties(static_cast<const T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		NewPropertySet diffProperties(const T& instance, const T& other, const NewPropertySet& includedProperties) const
		{
			return instance._diffProperties(other, includedProperties);
		}
	};
}