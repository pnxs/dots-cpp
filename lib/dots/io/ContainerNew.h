#pragma once
#include <map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots
{
	template <typename = type::Struct>
	struct ContainerNew;

	template <>
	struct ContainerNew<type::Struct>
	{
		struct key_compare
		{
			using is_transparent = void;

			bool operator () (const type::Struct& lhs, const type::Struct& rhs) const
			{
				return lhs._less(rhs, lhs._keyProperties());
			}
		};

		using container_t = std::map<type::AnyStruct, DotsCloneInformation, key_compare>;
		using const_iterator_t = container_t::const_iterator;
		using value_t = container_t::value_type;
		using node_t = container_t::node_type;

		ContainerNew(const type::StructDescriptor& descriptor);
		ContainerNew(const ContainerNew& other) = default;
		ContainerNew(ContainerNew&& other) = default;
		~ContainerNew() = default;

		ContainerNew& operator = (const ContainerNew& rhs) = default;
		ContainerNew& operator = (ContainerNew&& rhs) = default;

		const type::StructDescriptor& descriptor() const;

		const_iterator_t begin() const;
		const_iterator_t end() const;

		const_iterator_t cbegin() const;
		const_iterator_t cend() const;

		bool empty() const;
		size_t size() const;

		const value_t* findClone(const type::Struct& instance) const;
		const value_t& getClone(const type::Struct& instance) const;

		const type::Struct* find(const type::Struct& instance) const;
		const type::Struct& get(const type::Struct& instance) const;

		const value_t& insert(const DotsHeader& header, const type::Struct& instance);
		node_t remove(const DotsHeader& header, const type::Struct& instance);

		void clear();

		void forEachClone(const std::function<void(const value_t&)>& f) const;
		void forEach(const std::function<void(const type::Struct&)>& f) const;

		size_t totalMemoryUsage() const;

		template <typename T>
		const ContainerNew<T>& as() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);

			if (&T::_Descriptor() != m_descriptor)
			{
				throw std::runtime_error{ "type mismatch: expected " + m_descriptor->name() + " but got " + T::_Descriptor().name() };
			}

			return static_cast<const ContainerNew<T>&>(*this);
		}

		template <typename T>
		ContainerNew<T>& as()
		{
			return const_cast<ContainerNew<T>&>(std::as_const(*this).as<T>());
		}

	private:

		const type::StructDescriptor* m_descriptor;
		container_t m_instances;
	};

	template <typename T>
	struct ContainerNew : ContainerNew<type::Struct>
	{
		static_assert(std::is_base_of_v<type::Struct, T>);

		ContainerNew() :
			ContainerNew<type::Struct>(T::_Descriptor())
		{
			/* do nothing */
		}
		ContainerNew(const ContainerNew& other) = default;
		ContainerNew(ContainerNew&& other) = default;
		~ContainerNew() = default;

		ContainerNew& operator = (const ContainerNew& rhs) = default;
		ContainerNew& operator = (ContainerNew&& rhs) = default;

		const T* find(const T& instance) const
		{
			return static_cast<const T*>(ContainerNew<>::find(instance));
		}

		const T& get(const T& instance) const
		{
			return static_cast<const T&>(ContainerNew<>::get(instance));
		}

		void forEach(const std::function<void(const T&)>& f) const
		{
			forEachClone([&](const value_t& value)
			{
				f(static_cast<const T&>(value.first));
			});
		}

	private:

		using ContainerNew<>::find;
		using ContainerNew<>::get;
		using ContainerNew<>::forEach;
		using ContainerNew<>::as;
	};
}