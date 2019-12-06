#pragma once
#include <map>
#include <memory>
#include <type_traits>
#include <dots/type/NewDescriptor.h>
#include <dots/type/NewFundamentalTypes.h>
#include <dots/type/NewEnumDescriptor.h>
#include <dots/type/NewStructDescriptor.h>

namespace dots::io
{
    struct NewRegistry
    {
        NewRegistry();
        NewRegistry(const NewRegistry& other) = default;
        NewRegistry(NewRegistry&& other) noexcept = default;
        ~NewRegistry() = default;

        NewRegistry& operator = (const NewRegistry& rhs) = default;
        NewRegistry& operator = (NewRegistry&& rhs) noexcept = default;

    	std::shared_ptr<type::NewDescriptor<>> findType(const std::string_view& name, bool assertNotNull = false) const;
    	std::shared_ptr<type::NewEnumDescriptor<>> findEnumType(const std::string_view& name, bool assertNotNull = false) const;
    	std::shared_ptr<type::NewStructDescriptor<>> findStructType(const std::string_view& name, bool assertNotNull = false) const;

    	const type::NewDescriptor<>& getType(const std::string_view& name) const;
    	const type::NewEnumDescriptor<>& getEnumType(const std::string_view& name) const;
    	const type::NewStructDescriptor<>& getStructType(const std::string_view& name) const;
    	
    	bool hasType(const std::string_view& name) const;

    	std::shared_ptr<type::NewDescriptor<>> registerType(std::shared_ptr<type::NewDescriptor<>> descriptor, bool assertNewType = true);

    	template <typename D, std::enable_if_t<std::is_base_of_v<type::NewDescriptor<>, D>, int> = 0>
    	std::shared_ptr<D> registerType(D&& descriptor)
    	{
    		return std::static_pointer_cast<D>(registerType(std::make_shared<D>(std::forward<D>(descriptor))));
    	}

    	void deregisterType(const std::shared_ptr<type::NewDescriptor<>>& descriptor, bool assertRegisteredType = true);
    	void deregisterType(const type::NewDescriptor<>& descriptor, bool assertRegisteredType = true);
    	void deregisterType(const std::string_view& name, bool assertRegisteredType = true);

    private:

    	std::map<std::string_view, std::shared_ptr<type::NewDescriptor<>>> m_types;
	};
}