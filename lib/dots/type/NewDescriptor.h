#pragma once
#include <string>
#include <memory>
#include <dots/type/NewTypeless.h>

namespace dots::type
{
	enum struct NewType : uint8_t
	{
	    boolean,
	    int8, uint8, int16, uint16, int32, uint32, int64, uint64,
	    float32, float64,
		property_set,
		timepoint, steady_timepoint, duration,
		uuid, string,
	    Vector,
		Struct, Enum
	};

	template <typename = NewTypeless>
	struct NewDescriptor;

	template <>
	struct NewDescriptor<NewTypeless>
	{
		NewDescriptor(NewType type, std::string name, size_t size, size_t alignment);
		NewDescriptor(const NewDescriptor& other) = default;
		NewDescriptor(NewDescriptor&& other) = default;
		virtual ~NewDescriptor() = default;

		NewDescriptor& operator = (const NewDescriptor& rhs) = default;
		NewDescriptor& operator = (NewDescriptor&& rhs) = default;

		NewType type() const;
		bool isFundamentalType() const;
		
		const std::string& name() const;
		size_t size() const;
		size_t alignment() const;

		virtual NewTypeless& construct(NewTypeless& value) const = 0;
		virtual NewTypeless& construct(NewTypeless& value, const NewTypeless& other) const = 0;
		virtual NewTypeless& construct(NewTypeless& value, NewTypeless&& other) const = 0;
	    virtual void destruct(NewTypeless& value) const = 0;

	    virtual NewTypeless& assign(NewTypeless& lhs, const NewTypeless& rhs) const = 0;
		virtual NewTypeless& assign(NewTypeless& lhs, NewTypeless&& rhs) const = 0;
	    virtual void swap(NewTypeless& value, NewTypeless& other) const = 0;

	    virtual bool equal(const NewTypeless& lhs, const NewTypeless& rhs) const = 0;
	    virtual bool less(const NewTypeless& lhs, const NewTypeless& rhs) const = 0;
		virtual bool lessEqual(const NewTypeless& lhs, const NewTypeless& rhs) const = 0;
		virtual bool greater(const NewTypeless& lhs, const NewTypeless& rhs) const = 0;
		virtual bool greaterEqual(const NewTypeless& lhs, const NewTypeless& rhs) const = 0;

		virtual bool usesDynamicMemory() const;
		virtual size_t dynamicMemoryUsage(const NewTypeless& value) const;

		static bool IsFundamentalType(const NewDescriptor& descriptor);
		static bool IsFundamentalType(NewType type);

	private:

		NewType m_type;
		std::string m_name;
		size_t m_size;
		size_t m_alignment;
	};

	template <typename T>
	struct is_new_descriptor : std::false_type {};

	template <typename T>
	struct is_new_descriptor<NewDescriptor<T>> : std::true_type {};

	template <typename T>
	using is_new_descriptor_t = typename is_new_descriptor<T>::type;

	template <typename T>
	constexpr bool is_new_descriptor_v = is_new_descriptor_t<T>::value;

	template <typename T>
	struct new_described_type
	{
		static_assert(is_new_descriptor_v<T>, "T has to be a descriptor");
	};

	template <typename T>
	struct new_described_type<NewDescriptor<T>>
	{
		static_assert(is_new_descriptor_v<NewDescriptor<T>>, "T has to be a descriptor");
		using type = T;
	};

	template <typename T>
	using new_described_type_t = typename new_described_type<T>::type;
}