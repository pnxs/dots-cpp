#pragma once
#include <dots/type/NewStaticDescriptor.h>
#include <dots/type/NewVector.h>

namespace dots::type
{
	struct NewVectorDescriptor : NewDescriptor<NewTypeless>
	{
		NewVectorDescriptor(std::string name, const NewDescriptor<NewTypeless>& valueDescriptor, size_t size, size_t alignment);
		NewVectorDescriptor(const NewVectorDescriptor& other) = default;
		NewVectorDescriptor(NewVectorDescriptor&& other) = default;
		~NewVectorDescriptor() = default;

		NewVectorDescriptor& operator = (const NewVectorDescriptor& rhs) = default;
		NewVectorDescriptor& operator = (NewVectorDescriptor&& rhs) = default;

		const NewDescriptor<NewTypeless>& valueDescriptor() const;

	private:

		const NewDescriptor<NewTypeless>* m_valueDescriptor;
	};

	template <typename T>
	struct NewDescriptor<NewVector<T>> : NewStaticDescriptor<NewVector<T>, NewVectorDescriptor>
	{
		NewDescriptor() :
			NewStaticDescriptor<NewVector<T>, NewVectorDescriptor>("vector<" + valueDescriptor().name() + ">", valueDescriptor(), sizeof(NewVector<T>), alignof(NewVector<T>))
		{
			/* do nothing */
		}

		bool usesDynamicMemory() const override
		{
			return true;
		}

		size_t dynamicMemoryUsage(const NewTypeless& lhs) const
		{
			return dynamicMemoryUsage(lhs.to<NewVector<T>>());
		}

		size_t dynamicMemoryUsage(const NewVector<T>& lhs) const
		{
			size_t size = lhs.size();
		    size_t dynMemUsage = size * valueDescriptor().size();

			if (valueDescriptor().usesDynamicMemory())
			{
				for (const T& value : lhs)
				{
					dynMemUsage += valueDescriptor().dynamicMemoryUsage(value);
				}
			}			

		    return dynMemUsage;
		}

		static const NewDescriptor<T>& valueDescriptor()
		{
			return NewDescriptor<T>::Instance();
		}

	private:

		using NewVectorDescriptor::valueDescriptor;
	};
}