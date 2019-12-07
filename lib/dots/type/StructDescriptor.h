#pragma once
#include <dots/type/Descriptor.h>
#include <dots/type/StaticDescriptor.h>
#include <dots/type/Property.h>

namespace dots::types
{
	struct StructDescriptorData;
}

namespace dots::type
{
	struct Struct;

	template <typename T = Typeless, typename = void>
	struct StructDescriptor;

	template <>
	struct StructDescriptor<Typeless> : Descriptor<Typeless>
	{
		static const uint8_t Uncached      = 0b0000'0000;
		static const uint8_t Cached        = 0b0000'0001;
		static const uint8_t Internal      = 0b0000'0010;
		static const uint8_t Persistent    = 0b0000'0100;
		static const uint8_t Cleanup       = 0b0000'1000;
		static const uint8_t Local         = 0b0001'0000;
		static const uint8_t SubstructOnly = 0b0010'0000;
		
		StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptors, size_t size, size_t alignment);
		StructDescriptor(const StructDescriptor& other) = default;
		StructDescriptor(StructDescriptor&& other) = default;
		~StructDescriptor() = default;

		StructDescriptor& operator = (const StructDescriptor& rhs) = default;
		StructDescriptor& operator = (StructDescriptor&& rhs) = default;

		Typeless& construct(Typeless& value) const override;
		Struct& construct(Struct& instance) const;
		Typeless& construct(Typeless& value, const Typeless& other) const override;
		Struct& construct(Struct& instance, const Struct& other) const;
		Typeless& construct(Typeless& value, Typeless&& other) const override;
		Struct& construct(Struct& instance, Struct&& other) const;
		
		void destruct(Typeless& value) const override;
		Struct& destruct(Struct& instance) const;
		
		Typeless& assign(Typeless& lhs, const Typeless& rhs) const override;
		Typeless& assign(Typeless& lhs, Typeless&& rhs) const override;
		void swap(Typeless& value, Typeless& other) const override;
		
		bool equal(const Typeless& lhs, const Typeless& rhs) const override;
		bool less(const Typeless& lhs, const Typeless& rhs) const override;
		bool lessEqual(const Typeless& lhs, const Typeless& rhs) const override;
		bool greater(const Typeless& lhs, const Typeless& rhs) const override;
		bool greaterEqual(const Typeless& lhs, const Typeless& rhs) const override;

		bool usesDynamicMemory() const override;
		size_t dynamicMemoryUsage(const Typeless& instance) const override;
		size_t dynamicMemoryUsage(const Struct& instance) const;

		virtual Struct& assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const = 0;
		virtual Struct& copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const = 0;
		virtual Struct& merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const = 0;
		virtual void swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const = 0;
		virtual void clear(Struct& instance, const PropertySet& includedProperties) const = 0;

		virtual bool equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const = 0;
		virtual bool same(const Struct& lhs, const Struct& rhs) const = 0;
		
		virtual bool less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const = 0;
		virtual bool lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const = 0;
		virtual bool greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const = 0;
		virtual bool greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const = 0;

		virtual PropertySet diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const = 0;

		virtual const PropertyArea& propertyArea(const Struct& instance) const = 0;
		virtual PropertyArea& propertyArea(Struct& instance) const = 0;

		uint8_t flags() const;
		bool cached() const;
		bool cleanup() const;
		bool local() const;
		bool persistent() const;
		bool internal() const;
		bool substructOnly() const;

		const property_descriptor_container_t& propertyDescriptors() const;
		
		const PropertySet& properties() const;
		const PropertySet& keyProperties() const;

		[[deprecated("only available for backwards compatibility")]]
		const PropertySet& keys() const;

		[[deprecated("only available for backwards compatibility")]]
		const PropertySet& validProperties(const void* instance) const;

		[[deprecated("only available for backwards compatibility")]]
		PropertySet& validProperties(void* instance) const;

		[[deprecated("only available for backwards compatibility")]]
		const types::StructDescriptorData& descriptorData() const;

		[[deprecated("only available for backwards compatibility")]]
		static const StructDescriptor<>* createFromStructDescriptorData(const types::StructDescriptorData& sd);

	private:

		uint8_t m_flags;
		property_descriptor_container_t m_propertyDescriptors;
		PropertySet m_properties;
		PropertySet m_keyProperties;
		PropertySet m_dynamicMemoryProperties;
		mutable const types::StructDescriptorData* m_descriptorData = nullptr;
	};

	template <typename T>
	struct StructDescriptor<T> : StaticDescriptor<T, StructDescriptor<Typeless>>
	{
		static_assert(std::is_base_of_v<Struct, T>);
		
		StructDescriptor(std::string name, uint8_t flags, const property_descriptor_container_t& propertyDescriptor) :
			StaticDescriptor<T, StructDescriptor<Typeless>>(std::move(name), flags, propertyDescriptor, sizeof(T), alignof(T))
		{
			/* do nothing */
		}
		StructDescriptor(const StructDescriptor& other) = default;
		StructDescriptor(StructDescriptor&& other) = default;
		~StructDescriptor() = default;

		StructDescriptor& operator = (const StructDescriptor& rhs) = default;
		StructDescriptor& operator = (StructDescriptor&& rhs) = default;

		using StaticDescriptor<T, StructDescriptor<Typeless>>::assign;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::swap;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::equal;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::less;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::lessEqual;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::greater;
		using StaticDescriptor<T, StructDescriptor<Typeless>>::greaterEqual;

		Struct& assign(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
		{
			return assign(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& assign(T& instance, const T& other, const PropertySet& includedProperties) const
		{
			return instance._assign(other, includedProperties);
		}

		Struct& copy(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
		{
			return copy(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& copy(T& instance, const T& other, const PropertySet& includedProperties) const
		{
			return instance._copy(other, includedProperties);
		}

		Struct& merge(Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
		{
			return merge(static_cast<T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		T& merge(T& instance, const T& other, const PropertySet& includedProperties) const
		{
			return instance._merge(other, includedProperties);
		}
		
		void swap(Struct& instance, Struct& other, const PropertySet& includedProperties) const override
		{
			swap(static_cast<T&>(instance), static_cast<T&>(other), includedProperties);
		}

		void swap(T& instance, T& other, const PropertySet& includedProperties) const
		{
			instance._swap(other, includedProperties);
		}

		void clear(Struct& instance, const PropertySet& includedProperties) const override
		{
			clear(static_cast<T&>(instance), includedProperties);
		}

		void clear(T& instance, const PropertySet& includedProperties) const
		{
			instance._clear(includedProperties);
		}

		bool equal(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
		{
			return equal(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool equal(const T& lhs, const T& rhs, const PropertySet& includedProperties) const
		{
			return lhs._equal(rhs, includedProperties);
		}

		bool same(const Struct& lhs, const Struct& rhs) const override
		{
			return same(static_cast<const T&>(lhs), static_cast<const T&>(rhs));
		}

		bool same(const T& lhs, const T& rhs) const
		{
			return lhs._same(rhs);
		}

		bool less(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
		{
			return less(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool less(const T& lhs, const T& rhs, const PropertySet& includedProperties) const
		{
			return lhs._less(rhs, includedProperties);
		}

		bool lessEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
		{
			return lessEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool lessEqual(const T& lhs, const T& rhs, const PropertySet& includedProperties) const
		{
			return lhs._lessEqual(rhs, includedProperties);
		}

		bool greater(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
		{
			return greater(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool greater(const T& lhs, const T& rhs, const PropertySet& includedProperties) const
		{
			return lhs._greater(rhs, includedProperties);
		}

		bool greaterEqual(const Struct& lhs, const Struct& rhs, const PropertySet& includedProperties) const override
		{
			return greaterEqual(static_cast<const T&>(lhs), static_cast<const T&>(rhs), includedProperties);
		}

		bool greaterEqual(const T& lhs, const T& rhs, const PropertySet& includedProperties) const
		{
			return lhs._greaterEqual(rhs, includedProperties);
		}

		PropertySet diffProperties(const Struct& instance, const Struct& other, const PropertySet& includedProperties) const override
		{
			return diffProperties(static_cast<const T&>(instance), static_cast<const T&>(other), includedProperties);
		}

		PropertySet diffProperties(const T& instance, const T& other, const PropertySet& includedProperties) const
		{
			return instance._diffProperties(other, includedProperties);
		}

		const PropertyArea& propertyArea(const Struct& instance) const override
		{
			return propertyArea(static_cast<const T&>(instance));
		}

		const PropertyArea& propertyArea(const T& instance) const
		{
			return instance._propertyArea();
		}

		PropertyArea& propertyArea(Struct& instance) const override
		{
			return propertyArea(static_cast<T&>(instance));
		}

		PropertyArea& propertyArea(T& instance) const
		{
			return instance._propertyArea();
		}
	};

	[[deprecated("only available for backwards compatibility")]]
	inline const StructDescriptor<>* toStructDescriptor(const Descriptor<>* descriptor)
	{
		return descriptor->type() == Type::Struct ? static_cast<const StructDescriptor<>*>(descriptor) : nullptr;
	}
}