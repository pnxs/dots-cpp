#pragma once
#include <vector>
#include <set>
#include <optional>
#include "property_set.h"
#include "StructProperty.h"
#include "Descriptor.h"

namespace dots::type
{
	struct Struct;
}

namespace dots::types
{
	struct StructDescriptorData;
}

namespace dots::type
{
	class StructDescriptor : public Descriptor
	{
	public:
		typedef dots::types::StructDescriptorData DescriptorData;

		StructDescriptor(const DescriptorData& sd);

		static const StructDescriptor* createFromStructDescriptorData(const DescriptorData& sd);

		void construct(void*) const final override;
		void destruct(void*) const final override;

		bool usesDynamicMemory() const override;
		size_t dynamicMemoryUsage(const void* lhs) const override;

		virtual std::string to_string(const void* lhs) const final override;
		virtual bool from_string(void* lhs, const std::string& str) const final override;

		bool equal(const void* lhs, const void* rhs) const final override;
		bool lessThan(const void* lhs, const void* rhs) const final override;
		void copy(void* lhs, const void* rhs) const final override;
		void swap(void* lhs, void* rhs) const final override;
		void clear(void* lhs) const final override;

		void copy(void* lhs, const void* rhs, property_set properties) const;
		void swap(void* lhs, void* rhs, property_set properties) const;
		void clear(void* lhs, property_set properties) const;
		property_set diff(const void* lhs, const void* rhs, property_set properties) const;
		void merge(void* lhs, const void* rhs, const void* properties) const;

		const property_set& keys() const;

		property_set& validProperties(const void* obj) const;
		const property_set& propertySet() const;

		const std::vector<StructProperty>& properties() const { return m_properties; }
		const DescriptorData& descriptorData() const;

		uint32_t publisherId() const { return m_publisherId; }

		bool cached() const;
		bool cleanup() const;
		bool local() const;
		bool persistent() const;
		bool internal() const;
		bool substructOnly() const;

	private:

		size_t determineSize(const types::StructDescriptorData &sd);
		size_t determineAlignment(const types::StructDescriptorData& sd);
		size_t determineOffset(const Descriptor* td, size_t start);

		std::vector<StructProperty> m_properties;
		property_set m_propertySet;
		property_set m_keyProperties;
		std::unique_ptr<DescriptorData> m_descriptorData;
		uint32_t m_publisherId = 0;
		mutable std::optional<std::vector<const StructProperty*>> m_dynamicMemoryProperties;
	};

	inline const StructDescriptor* toStructDescriptor(const Descriptor* d)
	{
		return dynamic_cast<const StructDescriptor*>(d);
	}

	struct TypeNameLessThan
	{
		bool operator() (const StructDescriptor* lhs, const StructDescriptor* rhs) const
		{
			return lhs->name() < rhs->name();
		}
	};

	class StructDescriptorSet : public std::set<const StructDescriptor*, TypeNameLessThan>
	{
		typedef std::set<const StructDescriptor*, TypeNameLessThan> base_type;

	public:
		template<class T>
		void insert()
		{
			base_type::insert(T::_td());
		}

		using base_type::insert;

		template<class T>
		void erase()
		{
			base_type::erase(T::_td());
		}

		using base_type::erase;

		void merge(const StructDescriptorSet& rhs);
	};
}