#pragma once
#include <dots/type/NewStruct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots
{
    template<typename = type::NewStruct>
    struct Event;

    template<>
    struct Event<type::NewStruct>
    {
        Event(const DotsHeader& header, const type::NewStruct& transmitted, const type::NewStruct& updated, const DotsCloneInformation& cloneInfo);
		Event(const Event& other) = delete;
		Event(Event&& other) = delete;
		~Event() = default;

		Event& operator = (const Event& rhs) = delete;
		Event& operator = (Event&& rhs) = delete;

        const type::NewStruct& operator () () const;

        const DotsHeader& header() const;
        const type::NewStruct& transmitted() const;
        const type::NewStruct& updated() const;
        const DotsCloneInformation& cloneInfo() const;

		const type::NewStructDescriptor<>& descriptor() const;

        DotsMt mt() const;
        bool isCreate() const;
        bool isUpdate() const;
        bool isRemove() const;

        bool isOwnUpdate() const;		
	    types::property_set_t newProperties() const { return header().attributes; }
	    types::property_set_t updatedProperties() const { return newProperties() ^ updated()._validProperties(); }

        template <typename T>
        const Event<T>& as() const
        {
            static_assert(std::is_base_of_v<type::NewStruct, T>);

            if (&T::_Descriptor() != &m_transmitted._descriptor())
            {
                throw std::runtime_error{ "type mismatch: expected " + m_transmitted._descriptor().name() + " but got " + T::_Descriptor().name() };
            }

            return static_cast<const Event<T>&>(*this);
        }

        template <typename T>
        Event<T>& as()
        {
            return const_cast<Event<T>&>(std::as_const(*this).as<T>());
        }

    private:

        const DotsHeader& m_header;
        const type::NewStruct& m_transmitted;
        const type::NewStruct& m_updated;
        const DotsCloneInformation m_cloneInfo;
    };

    template<typename T>
    struct Event : Event<type::NewStruct>
    {
        static_assert(std::is_base_of_v<type::NewStruct, T>);

        Event(const DotsHeader& header, const T& transmitted, const T& updated, const DotsCloneInformation& cloneInfo) :
            Event<type::NewStruct>(header, transmitted, updated, cloneInfo)
        {
            /* do nothing */
        }
		Event(const Event& other) = delete;
		Event(Event&& other) = delete;
		~Event() = default;

		Event& operator = (const Event& rhs) = delete;
		Event& operator = (Event&& rhs) = delete;

		const T& operator () () const
		{
			return static_cast<const T&>(Event<type::NewStruct>::operator()());
		}

        const T& transmitted() const
        {
            return static_cast<const T&>(Event<type::NewStruct>::transmitted());
        }

        const T& updated() const
        {
            return static_cast<const T&>(Event<type::NewStruct>::updated());
        }

    private:

		using Event<type::NewStruct>::operator();
        using Event<type::NewStruct>::transmitted;
        using Event<type::NewStruct>::updated;
    };
}