#pragma once
#include <dots/type/Struct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots
{
    template<typename = type::Struct>
    struct Event;

    template<>
    struct Event<type::Struct>
    {
        Event(const DotsHeader& header, const type::Struct& transmitted, const type::Struct& updated, const DotsCloneInformation& cloneInfo);
		Event(const Event& other) = delete;
		Event(Event&& other) = delete;
		~Event() = default;

		Event& operator = (const Event& rhs) = delete;
		Event& operator = (Event&& rhs) = delete;

        const type::Struct& operator () () const;

        const DotsHeader& header() const;
        const type::Struct& transmitted() const;
        const type::Struct& updated() const;
        const DotsCloneInformation& cloneInfo() const;

		const type::StructDescriptor& descriptor() const;

        DotsMt mt() const;
        bool isCreate() const;
        bool isUpdate() const;
        bool isRemove() const;

        bool isOwnUpdate() const;		
	    property_set newProperties() const { return header().attributes; }
	    property_set updatedProperties() const { return newProperties() & updated()._validProperties(); }

        template <typename T>
        const Event<T>& as() const
        {
            static_assert(std::is_base_of_v<type::Struct, T>);

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
        const type::Struct& m_transmitted;
        const type::Struct& m_updated;
        const DotsCloneInformation m_cloneInfo;
    };

    template<typename T>
    struct Event : Event<type::Struct>
    {
        static_assert(std::is_base_of_v<type::Struct, T>);

        Event(const DotsHeader& header, const T& transmitted, const T& updated, const DotsCloneInformation& cloneInfo) :
            Event<type::Struct>(header, transmitted, updated, cloneInfo)
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
			return static_cast<const T&>(Event<type::Struct>::operator()());
		}

        const T& transmitted() const
        {
            return static_cast<const T&>(Event<type::Struct>::transmitted());
        }

        const T& updated() const
        {
            return static_cast<const T&>(Event<type::Struct>::updated());
        }

    private:

		using Event<type::Struct>::operator();
        using Event<type::Struct>::transmitted;
        using Event<type::Struct>::updated;
    };
}