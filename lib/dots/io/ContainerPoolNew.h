#pragma once
#include <map>
#include <string_view>
#include <dots/io/ContainerNew.h>

namespace dots
{
    struct ContainerPoolNew
    {
        using pool_t = std::map<const type::StructDescriptor*, ContainerNew<>>;
		using iterator_t = pool_t::iterator;
        using const_iterator_t = pool_t::const_iterator;
        using value_t = pool_t::value_type;
        using node_t = pool_t::node_type;

		ContainerPoolNew() = default;
		ContainerPoolNew(const ContainerPoolNew& other) = default;
		ContainerPoolNew(ContainerPoolNew&& other) = default;
		~ContainerPoolNew() = default;

		ContainerPoolNew& operator = (const ContainerPoolNew& rhs) = default;
		ContainerPoolNew& operator = (ContainerPoolNew&& rhs) = default;

        const_iterator_t begin() const;
        const_iterator_t end() const;

		iterator_t begin();
		iterator_t end();

        const_iterator_t cbegin() const;
        const_iterator_t cend() const;

		bool empty() const;
		size_t size() const;

        const ContainerNew<>* find(const type::StructDescriptor& descriptor) const;
        const ContainerNew<>& get(const type::StructDescriptor& descriptor) const;

		ContainerNew<>* find(const type::StructDescriptor& descriptor);
		ContainerNew<>& get(const type::StructDescriptor& descriptor, bool insertIfNotExist = true);

		const ContainerNew<>* find(const std::string_view& name) const;
        const ContainerNew<>& get(const std::string_view& name) const;

		ContainerNew<>* find(const std::string_view& name);
		ContainerNew<>& get(const std::string_view& name);

        node_t remove(const type::StructDescriptor& descriptor);

        void forEach(const std::function<void(const ContainerNew<>&)>& f) const;

		size_t totalMemoryUsage() const;

		template <typename T>
		const ContainerNew<T>* find() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<const ContainerNew<T>*>(find(T::_Descriptor()));
		}

		template <typename T>
		const ContainerNew<T>& get() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<const ContainerNew<T>&>(get(T::_Descriptor()));
		}

		template <typename T>
		ContainerNew<T>* find()
		{
			return const_cast<ContainerNew<T>*>(static_cast<const ContainerNew<T>*>((std::as_const(*this).find<T>())));
		}

		template <typename T>
		ContainerNew<T>& get(bool insertIfNotExist = true)
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<ContainerNew<T>&>(get(T::_Descriptor(), insertIfNotExist));
		}

    private:

		using name_cache_t = std::map<std::string, ContainerNew<>*, std::less<>>;

        pool_t m_pool;
		name_cache_t m_nameCache;
    };
}