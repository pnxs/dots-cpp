#pragma once
#include <dots/type/NewStaticDescriptor.h>
#include <dots/type/NewPropertySet.h>
#include <dots/common/Chrono.h>
#include <dots/Uuid.h>
#include <dots/type/NewVector.h>
#include <dots/type/NewVectorDescriptor.h>

namespace dots::types
{
	using bool_t             = bool;

	using int8_t             = std::int8_t;
	using uint8_t            = std::uint8_t;
	using int16_t            = std::int16_t;
	using uint16_t           = std::uint16_t;
	using int32_t            = std::int32_t;
	using uint32_t           = std::uint32_t;
	using int64_t            = std::int64_t;
	using uint64_t           = std::uint64_t;

	using float32_t          = float;
	using float64_t          = double;

	using property_set_t     = type::PropertySet;

	using timepoint_t        = pnxs::chrono::TimePoint;
	using steady_timepoint_t = pnxs::chrono::SteadyTimePoint;
	using duration_t         = pnxs::chrono::Duration;

	using uuid_t             = uuid;
	using string_t           = std::string;

	template <typename T>
	using vector_t           = type::Vector<T>;
}

namespace dots::type
{	
	template <>
	struct Descriptor<types::bool_t> : StaticDescriptor<types::bool_t>
	{
		Descriptor() : StaticDescriptor(Type::boolean, "bool") {}
	};

	template <>
	struct Descriptor<types::int8_t> : StaticDescriptor<types::int8_t>
	{
		Descriptor() : StaticDescriptor(Type::int8, "int8") {}
	};

	template <>
	struct Descriptor<types::uint8_t> : StaticDescriptor<types::uint8_t>
	{
		Descriptor() : StaticDescriptor(Type::uint8, "uint8") {}
	};

	template <>
	struct Descriptor<types::int16_t> : StaticDescriptor<types::int16_t>
	{
		Descriptor() : StaticDescriptor(Type::int16, "int16") {}
	};

	template <>
	struct Descriptor<types::uint16_t> : StaticDescriptor<types::uint16_t>
	{
		Descriptor() : StaticDescriptor(Type::uint16, "uint16") {}
	};

	template <>
	struct Descriptor<types::int32_t> : StaticDescriptor<types::int32_t>
	{
		Descriptor() : StaticDescriptor(Type::int32, "int32") {}
	};

	template <>
	struct Descriptor<types::uint32_t> : StaticDescriptor<types::uint32_t>
	{
		Descriptor() : StaticDescriptor(Type::uint32, "uint32") {}
	};

	template <>
	struct Descriptor<types::int64_t> : StaticDescriptor<types::int64_t>
	{
		Descriptor() : StaticDescriptor(Type::int64, "int64") {}
	};

	template <>
	struct Descriptor<types::uint64_t> : StaticDescriptor<types::uint64_t>
	{
		Descriptor() : StaticDescriptor(Type::uint64, "uint64") {}
	};
	
	template <>
	struct Descriptor<types::float32_t> : StaticDescriptor<types::float32_t>
	{
		Descriptor() : StaticDescriptor(Type::float32, "float32") {}
	};

	template <>
	struct Descriptor<types::float64_t> : StaticDescriptor<types::float64_t>
	{
		Descriptor() : StaticDescriptor(Type::float64, "float64") {}
	};

	template <>
	struct Descriptor<types::property_set_t> : StaticDescriptor<types::property_set_t>
	{
		Descriptor() : StaticDescriptor(Type::property_set, "property_set") {}
	};

	template <>
	struct Descriptor<types::timepoint_t> : StaticDescriptor<types::timepoint_t>
	{
		Descriptor() : StaticDescriptor(Type::timepoint, "timepoint") {}
	};

	template <>
	struct Descriptor<types::steady_timepoint_t> : StaticDescriptor<types::steady_timepoint_t>
	{
		Descriptor() : StaticDescriptor(Type::steady_timepoint, "steady_timepoint") {}
	};

	template <>
	struct Descriptor<types::duration_t> : StaticDescriptor<types::duration_t>
	{
		Descriptor() : StaticDescriptor(Type::duration, "duration") {}
	};

	template <>
	struct Descriptor<types::uuid_t> : StaticDescriptor<types::uuid_t>
	{
		Descriptor() : StaticDescriptor(Type::uuid, "uuid") {}
	};

	template <>
	struct Descriptor<types::string_t> : StaticDescriptor<types::string_t>
	{
		Descriptor() : StaticDescriptor(Type::string, "string") {}

		bool usesDynamicMemory() const override
		{
			return true;
		}

		size_t dynamicMemoryUsage(const Typeless& value) const override
		{
			return dynamicMemoryUsage(value.to<types::string_t>());
		}

		size_t dynamicMemoryUsage(const types::string_t& value) const
		{
	        return value.empty() ? 0 : value.size() + 1;
		}
	};
}