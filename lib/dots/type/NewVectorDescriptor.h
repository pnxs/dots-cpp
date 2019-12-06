#pragma once
#include <dots/type/NewStaticDescriptor.h>
#include <dots/type/NewVector.h>

namespace dots::type
{
	struct NewVectorDescriptor : NewDescriptor<NewTypeless>
	{
		NewVectorDescriptor(std::string name, const std::shared_ptr<NewDescriptor<>>& valueDescriptor, size_t size, size_t alignment);
		NewVectorDescriptor(const NewVectorDescriptor& other) = default;
		NewVectorDescriptor(NewVectorDescriptor&& other) = default;
		~NewVectorDescriptor() = default;

		NewVectorDescriptor& operator = (const NewVectorDescriptor& rhs) = default;
		NewVectorDescriptor& operator = (NewVectorDescriptor&& rhs) = default;

		const std::shared_ptr<NewDescriptor<>>& valueDescriptorPtr() const;
		const NewDescriptor<NewTypeless>& valueDescriptor() const;

	private:

		std::shared_ptr<NewDescriptor<>> m_valueDescriptor;
	};

	template <typename T>
	struct NewDescriptor<NewVector<T>> : NewStaticDescriptor<NewVector<T>, NewVectorDescriptor>
	{
		NewDescriptor() :
			NewStaticDescriptor<NewVector<T>, NewVectorDescriptor>("vector<" + valueDescriptor().name() + ">", valueDescriptorPtr(), sizeof(NewVector<T>), alignof(NewVector<T>))
		{
			/* do nothing */
		}
		NewDescriptor(const std::shared_ptr<NewDescriptor<>>& valueDescriptorOverride) :
			NewStaticDescriptor<NewVector<T>, NewVectorDescriptor>("vector<" + valueDescriptorOverride->name() + ">", valueDescriptorOverride, sizeof(NewVector<T>), alignof(NewVector<T>))
		{
			if (valueDescriptorOverride->size() != valueDescriptor().size() || valueDescriptorOverride->alignment() != valueDescriptor().alignment())
			{
				throw std::logic_error{ "attempt to create vector descriptor with incompatible value type" };
			}
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

		static const std::shared_ptr<NewDescriptor<T>>& valueDescriptorPtr()
		{
			return NewDescriptor<T>::InstancePtr();
		}

		static const NewDescriptor<T>& valueDescriptor()
		{
			return NewDescriptor<T>::Instance();
		}

	private:

		using NewVectorDescriptor::valueDescriptorPtr;
		using NewVectorDescriptor::valueDescriptor;
	};
}