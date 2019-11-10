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

	using property_set_t     = type::NewPropertySet;

	using timepoint_t        = pnxs::chrono::TimePoint;
	using steady_timepoint_t = pnxs::chrono::SteadyTimePoint;
	using duration_t         = pnxs::chrono::Duration;

	using uuid_t             = uuid;
	using string_t           = std::string;

	template <typename T>
	using vector_t           = type::NewVector<T>;
}

namespace dots::type
{	
	template <>
	struct NewDescriptor<types::bool_t> : NewStaticDescriptor<types::bool_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::boolean, "bool") {}
	};

	template <>
	struct NewDescriptor<types::int8_t> : NewStaticDescriptor<types::int8_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::int8, "int8") {}
	};

	template <>
	struct NewDescriptor<types::uint8_t> : NewStaticDescriptor<types::uint8_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::uint8, "uint8") {}
	};

	template <>
	struct NewDescriptor<types::int16_t> : NewStaticDescriptor<types::int16_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::int16, "int16") {}
	};

	template <>
	struct NewDescriptor<types::uint16_t> : NewStaticDescriptor<types::uint16_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::uint16, "uint16") {}
	};

	template <>
	struct NewDescriptor<types::int32_t> : NewStaticDescriptor<types::int32_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::int32, "int32") {}
	};

	template <>
	struct NewDescriptor<types::uint32_t> : NewStaticDescriptor<types::uint32_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::uint32, "uint32") {}
	};

	template <>
	struct NewDescriptor<types::int64_t> : NewStaticDescriptor<types::int64_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::int64, "int64") {}
	};

	template <>
	struct NewDescriptor<types::uint64_t> : NewStaticDescriptor<types::uint64_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::uint64, "uint64") {}
	};
	
	template <>
	struct NewDescriptor<types::float32_t> : NewStaticDescriptor<types::float32_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::float32, "float32") {}
	};

	template <>
	struct NewDescriptor<types::float64_t> : NewStaticDescriptor<types::float64_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::float64, "float64") {}
	};

	template <>
	struct NewDescriptor<types::property_set_t> : NewStaticDescriptor<types::property_set_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::property_set, "property_set") {}
	};

	template <>
	struct NewDescriptor<types::timepoint_t> : NewStaticDescriptor<types::timepoint_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::timepoint, "timepoint") {}
	};

	template <>
	struct NewDescriptor<types::steady_timepoint_t> : NewStaticDescriptor<types::steady_timepoint_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::steady_timepoint, "steady_timepoint") {}
	};

	template <>
	struct NewDescriptor<types::duration_t> : NewStaticDescriptor<types::duration_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::duration, "duration") {}
	};

	template <>
	struct NewDescriptor<types::uuid_t> : NewStaticDescriptor<types::uuid_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::uuid, "uuid") {}
	};

	template <>
	struct NewDescriptor<types::string_t> : NewStaticDescriptor<types::string_t>
	{
		NewDescriptor() : NewStaticDescriptor(NewType::string, "string") {}

		bool usesDynamicMemory() const override
		{
			return true;
		}

		size_t dynamicMemoryUsage(const NewTypeless& value) const override
		{
			return dynamicMemoryUsage(value.to<types::string_t>());
		}

		size_t dynamicMemoryUsage(const types::string_t& value) const
		{
	        return value.empty() ? 0 : value.size() + 1;
		}
	};
}