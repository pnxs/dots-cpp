#pragma once
#include <map>
#include <unordered_map>
#include <string_view>
#include <dots/io/Container.h>

namespace dots
{
    struct ContainerPool
    {
        using pool_t = std::unordered_map<const type::StructDescriptor*, Container<>>;
		using iterator_t = pool_t::iterator;
        using const_iterator_t = pool_t::const_iterator;
        using value_t = pool_t::value_type;
        using node_t = pool_t::node_type;

		ContainerPool() = default;
		ContainerPool(const ContainerPool& other) = default;
		ContainerPool(ContainerPool&& other) = default;
		~ContainerPool() = default;

		ContainerPool& operator = (const ContainerPool& rhs) = default;
		ContainerPool& operator = (ContainerPool&& rhs) = default;

        const_iterator_t begin() const;
        const_iterator_t end() const;

		iterator_t begin();
		iterator_t end();

        const_iterator_t cbegin() const;
        const_iterator_t cend() const;

		bool empty() const;
		size_t size() const;

        const Container<>* find(const type::StructDescriptor& descriptor) const;
        const Container<>& get(const type::StructDescriptor& descriptor, bool insertIfNotExist = true) const;

		Container<>* find(const type::StructDescriptor& descriptor);
		Container<>& get(const type::StructDescriptor& descriptor, bool insertIfNotExist = true);

		const Container<>* find(const std::string_view& name) const;
        const Container<>& get(const std::string_view& name) const;

		Container<>* find(const std::string_view& name);
		Container<>& get(const std::string_view& name);

        node_t remove(const type::StructDescriptor& descriptor);

        void forEach(const std::function<void(const Container<>&)>& f) const;

		size_t totalMemoryUsage() const;

		template <typename T>
		const Container<T>* find() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<const Container<T>*>(find(T::_Descriptor()));
		}

		template <typename T>
		const Container<T>& get() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<const Container<T>&>(get(T::_Descriptor()));
		}

		template <typename T>
		Container<T>* find()
		{
			return const_cast<Container<T>*>(static_cast<const Container<T>*>((std::as_const(*this).find<T>())));
		}

		template <typename T>
		Container<T>& get(bool insertIfNotExist = true)
		{
			static_assert(std::is_base_of_v<type::Struct, T>);
			return static_cast<Container<T>&>(get(T::_Descriptor(), insertIfNotExist));
		}

    private:

		using name_cache_t = std::map<std::string, Container<>*, std::less<>>;

		// TODO: remove mutability when utilities are fixed to no longer require non-const pool access
        mutable pool_t m_pool;
		mutable name_cache_t m_nameCache;
    };
}