#pragma once
#include <map>
#include <functional>
#include <dots/type/AnyStruct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots::io
{
	template <typename = type::Struct>
	struct Container;

	template <>
	struct Container<type::Struct>
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

		Container(const type::StructDescriptor<>& descriptor);
		Container(const Container& other) = default;
		Container(Container&& other) = default;
		~Container() = default;

		Container& operator = (const Container& rhs) = default;
		Container& operator = (Container&& rhs) = default;

		const type::StructDescriptor<>& descriptor() const;

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
		const Container<T>& as() const
		{
			static_assert(std::is_base_of_v<type::Struct, T>);

			if (&T::_Descriptor() != m_descriptor)
			{
				throw std::runtime_error{ "type mismatch: expected " + m_descriptor->name() + " but got " + T::_Descriptor().name() };
			}

			return static_cast<const Container<T>&>(*this);
		}

		template <typename T>
		Container<T>& as()
		{
			return const_cast<Container<T>&>(std::as_const(*this).as<T>());
		}

	private:

		const type::StructDescriptor<>* m_descriptor;
		container_t m_instances;
	};

	template <typename T>
	struct Container : Container<type::Struct>
	{
		static_assert(std::is_base_of_v<type::Struct, T>);

		Container() :
			Container<type::Struct>(T::_Descriptor())
		{
			/* do nothing */
		}
		Container(const Container& other) = default;
		Container(Container&& other) = default;
		~Container() = default;

		Container& operator = (const Container& rhs) = default;
		Container& operator = (Container&& rhs) = default;

		const T* find(const T& instance) const
		{
			return static_cast<const T*>(Container<>::find(instance));
		}

		const T& get(const T& instance) const
		{
			return static_cast<const T&>(Container<>::get(instance));
		}

		void forEach(const std::function<void(const T&)>& f) const
		{
			forEachClone([&](const value_t& value)
			{
				f(value.first.to<T>());
			});
		}

	private:

		using Container<>::find;
		using Container<>::get;
		using Container<>::forEach;
		using Container<>::as;
	};
}