#pragma once
#include <string>
#include <string_view>
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

	// [[deprecated("only available for backwards compatibility")]]
	using DotsType = NewType;

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

		virtual void fromString(NewTypeless& storage, const std::string_view& value) const;
		virtual std::string toString(const NewTypeless& value) const;

		static bool IsFundamentalType(const NewDescriptor& descriptor);
		static bool IsFundamentalType(NewType type);

		[[deprecated("only available for backwards compatibility")]]
		DotsType dotsType() const
		{
			return type();
		}

		[[deprecated("only available for backwards compatibility")]]
		void* New() const
		{
		    void *obj = ::operator new(size());
		    construct(*NewTypeless::From(obj));
		    return obj;
		}

		[[deprecated("only available for backwards compatibility")]]
		void Delete(void *obj) const
		{
		    destruct(*NewTypeless::From(obj));
		    ::operator delete(obj);
		}

		[[deprecated("only available for backwards compatibility")]]
		std::shared_ptr<void> make_shared() const
		{
			return { New(), [this](void* obj){ Delete(obj); } };
		}

		[[deprecated("only available for backwards compatibility")]]
		std::string to_string(const void* lhs) const
		{
			return toString(*NewTypeless::From(lhs));
		}

		[[deprecated("only available for backwards compatibility")]]
		bool from_string(void* lhs, const std::string& str) const
		{
			try
			{
				fromString(*NewTypeless::From(lhs), str);
				return true;
			}
			catch (...)
			{
				return false;
			}
		}

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

	[[deprecated("only available for backwards compatibility and should be replaced by fundamental type check")]]
	inline bool isDotsBaseType(NewType dotsType)
	{
	    switch (dotsType)
	    {
	        case NewType::int8:
	        case NewType::int16:
	        case NewType::int32:
	        case NewType::int64:
	        case NewType::uint8:
	        case NewType::uint16:
	        case NewType::uint32:
	        case NewType::uint64:
	        case NewType::boolean:
	        case NewType::float32:
	        case NewType::float64:
	        case NewType::string:
	        case NewType::property_set:
	        case NewType::timepoint:
	        case NewType::steady_timepoint:
	        case NewType::duration:
	        case NewType::uuid:
	        case NewType::Enum:
	            return true;

	        case NewType::Vector:
	        case NewType::Struct:
	            return false;
	    }
		
	    return false;
	}
}