#pragma once
#include <string_view>
#include <array>
#include <functional>
#include "StructDescriptor.h"
#include "property_set.h"

namespace dots::types
{
	struct StructDescriptorData;
}

namespace dots::type
{
	struct StructDescription;
	struct PropertyDescription;

	struct property_iterator;
	struct const_property_iterator;
	struct reverse_property_iterator;
	struct const_reverse_property_iterator;

	struct property_pair_iterator;
	struct property_pair_iterator_const;
	struct const_property_pair_iterator_const;
	struct reverse_property_pair_iterator;
	struct reverse_property_pair_iterator_const;
	struct const_reverse_property_pair_iterator_const;

	struct property_range;
	struct const_property_range;
	struct reverse_property_range;
	struct const_reverse_property_range;

	struct property_pair_range;
	struct property_pair_range_const;
	struct const_property_pair_range_const;
	struct reverse_property_pair_range;
	struct reverse_property_pair_range_const;
	struct const_reverse_property_pair_range_const;

    struct Struct
    {
        Struct(const StructDescriptor& descriptor);
        Struct(const Struct& other);
        Struct(Struct&& other);
        ~Struct() = default;

        Struct& operator = (const Struct& rhs);
        Struct& operator = (Struct&& rhs);

        const StructDescriptor& _descriptor() const;

		property_set& _validPropertySet();
		const property_set& _validPropertySet() const;
		const property_set& _keyPropertySet() const;

		property_iterator _begin(const property_set& propertySet = PROPERTY_SET_ALL);
		const_property_iterator _begin(const property_set& propertySet = PROPERTY_SET_ALL) const;

		property_iterator _end(const property_set& propertySet = PROPERTY_SET_ALL);
		const_property_iterator _end(const property_set& propertySet = PROPERTY_SET_ALL) const;

		reverse_property_iterator _rbegin(const property_set& propertySet = PROPERTY_SET_ALL);
		const_reverse_property_iterator _rbegin(const property_set& propertySet = PROPERTY_SET_ALL) const;

		reverse_property_iterator _rend(const property_set& propertySet = PROPERTY_SET_ALL);
		const_reverse_property_iterator _rend(const property_set& propertySet = PROPERTY_SET_ALL) const;

		property_range _propertyRange(const property_set& propertySet = PROPERTY_SET_ALL);
		const_property_range _propertyRange(const property_set& propertySet = PROPERTY_SET_ALL) const;

		reverse_property_range _propertyRangeReversed(const property_set& propertySet = PROPERTY_SET_ALL);
		const_reverse_property_range _propertyRangeReversed(const property_set& propertySet = PROPERTY_SET_ALL) const;

		property_pair_range _propertyPairRange(Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL);
		property_pair_range_const _propertyPairRange(const Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL);
		const_property_pair_range_const _propertyPairRange(const Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL) const;

		reverse_property_pair_range _propertyPairRangeReversed(Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL);
		reverse_property_pair_range_const _propertyPairRangeReversed(const Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL);
		const_reverse_property_pair_range_const _propertyPairRangeReversed(const Struct& rhs, const property_set& propertySet = PROPERTY_SET_ALL) const;

		void _publish(const property_set& what = PROPERTY_SET_ALL, bool remove = false) const;
		void _remove(const property_set& what = PROPERTY_SET_ALL) const;

    protected:

		static constexpr uint8_t Uncached      = 0b0000'0000;
		static constexpr uint8_t Cached        = 0b0000'0001;
		static constexpr uint8_t Internal      = 0b0000'0010;
		static constexpr uint8_t Persistent    = 0b0000'0100;
		static constexpr uint8_t Cleanup       = 0b0000'1000;
		static constexpr uint8_t Local         = 0b0001'0000;
		static constexpr uint8_t SubstructOnly = 0b0010'0000;

		struct StructDescription
		{
			constexpr StructDescription(const std::string_view& name, uint8_t flags, const std::array<StructProperty, 32>& propertyDescriptions, size_t numProperties) :
				name(name), flags(flags), propertyDescriptions(propertyDescriptions), numProperties(numProperties) {}

			std::string_view name;
			uint8_t flags;
			std::array<StructProperty, 32> propertyDescriptions;
			size_t numProperties;
		};

		static const StructDescriptor* MakeStructDescriptor(StructDescriptor* structDescriptorAddr, const types::StructDescriptorData& structDescriptorData);
		static const StructDescriptor* MakeStructDescriptor(StructDescriptor* structDescriptorAddr, const StructDescription& structDescription);

    private:

		property_set _validPropSet;
        const StructDescriptor* _desc;        
    };

	property_iterator begin(Struct& instance);
	const_property_iterator begin(const Struct& instance);

	property_iterator end(Struct& instance);
	const_property_iterator end(const Struct& instance);

	reverse_property_iterator rbegin(Struct& instance);
	const_reverse_property_iterator rbegin(const Struct& instance);

	reverse_property_iterator rend(Struct& instance);
	const_reverse_property_iterator rend(const Struct& instance);
}