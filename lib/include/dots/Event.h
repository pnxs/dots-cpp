#pragma once
#include <optional>
#include <dots/type/Struct.h>
#include <DotsHeader.dots.h>
#include <DotsCloneInformation.dots.h>

namespace dots
{
    template<typename = type::Struct>
    struct Event;

    /*!
     * @brief Information about a specific DOTS event.
     *
     * Instances of this class contain information about a specific DOTS
     * event. Events occur when incoming transmissions are dispatched (i.e.
     * processed) by a Transceiver to registered event handlers (i.e.
     * subscribers) of a particular DOTS struct type.
     *
     * If the struct type is cached, events refer to the effective
     * operation after the local Container instance was updated. Uncached
     * types, which do not have a container, will always be presented as
     * DotsMt::create events to the user.
     *
     * @remark Unless otherwise required, users are advised to use the
     * explicitly typed versions of the Event<> class.
     *
     * @remark Event<> instances are read-only views of the event data and
     * are only valid during dispatch of the event. They are therefore
     * neither copyable nor movable.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Event<> objects themselves.
     */
    template<>
    struct Event<type::Struct>
    {
        /*!
         * @brief Construct a new Event<> object.
         *
         * Note that the Event<> will store only references to most of the
         * arguments. The referred objects must therefore stay valid until the
         * event has been processed.
         *
         * @param header The header contained in the transmission that
         * triggered the event. If the transmission was originally transmitted
         * from the current transceiver (i.e. the one that is processing this
         * event), the @p isFromMyself flag has to be set to true.
         *
         * @param transmitted The instance contained in the transmission that
         * triggered the event.
         *
         * @param updated The updated instance of the event. For cached types,
         * this refers to the instance in the local Container. Otherwise it
         * must be the same instance as @p transmitted .
         *
         * @param cloneInfo The clone information of the event. For cached
         * types, this refers to the clone info in the local Container.
         *
         * @param mt The operation (i.e. "type" or "category") of the event. If
         * no operation is given, the last operation of @p cloneInfo will be
         * used.
         */
        Event(const DotsHeader& header, const type::Struct& transmitted, const type::Struct& updated, const DotsCloneInformation& cloneInfo, std::optional<DotsMt> mt = std::nullopt);
        Event(const Event& other) = delete;
        Event(Event&& other) = delete;
        ~Event() = default;

        Event& operator = (const Event& rhs) = delete;
        Event& operator = (Event&& rhs) = delete;

        /*!
         * @brief Get the updated instance.
         *
         * Using this operator is short hand for Event<>::updated().
         *
         * For cached types, the returned instance refers to the instance in
         * the local Container. 
         *
         * For uncached types, this is equivalent to Event<>::transmitted().
         *
         * @return const type::Struct& A reference to the updated instance.
         */
        const type::Struct& operator () () const;

        /*!
         * @brief Get the transmitted header.
         *
         * This is the header as it was contained in the transmission that
         * triggered this event.
         *
         * If the transmission was originally transmitted from the current
         * transceiver (i.e. the one that is processing this event), the @p
         * isFromMyself will be be set to true.
         *
         * @return const type::Struct& A reference to the transmitted header.
         */
        const DotsHeader& header() const;

        /*!
         * @brief Get the transmitted instance.
         *
         * This is the unaltered instance as it was contained in the
         * transmission that triggered this event.
         *
         * @return const type::Struct& A reference to the transmitted instance.
         */
        const type::Struct& transmitted() const;

        /*!
         * @brief Get the updated instance.
         *
         * For cached types, the returned instance refers to the instance in
         * the local Container (i.e. the "local clone").
         *
         * For uncached types, this is equivalent to Event<>::transmitted().
         *
         * @return const type::Struct& A reference to the updated instance.
         */
        const type::Struct& updated() const;

        /*!
         * @brief Get the updated clone information.
         *
         * For cached types, the returned clone information refer to the
         * instance in the local Container (i.e. the "local clone").
         *
         * For uncached types, the clone info will refer to the transmission
         * itself and always have DotsMt::create as the last operation.
         *
         * @return const type::Struct& A reference to the updated clone
         * information.
         */
        const DotsCloneInformation& cloneInfo() const;

        /*!
         * @brief Get the descriptor of the DOTS struct type of the event.
         *
         * @return const type::StructDescriptor& A reference to the
         * descriptor of the DOTS struct type.
         */
        const type::StructDescriptor& descriptor() const;

        /*!
         * @brief Get the operation (i.e. "type" or "category") of the event.
         *
         * For cached types, this will refer to the last operation that was
         * performed on the local Container.
         *
         * For uncached types, this will always return DotsMt::create.
         *
         * @return DotsMt The operation of the event.
         */
        DotsMt mt() const;

        /*!
         * @brief Indicates whether the event is a "create" event.
         *
         * This is equivalent to comparing Event<>::mt() against
         * DotsMt::create.
         *
         * Note that this is always true for uncached types.
         *
         * @return true If the event is a "create" event (i.e. Event<>::mt() ==
         * DotsMt::create is true).
         * @return false Else.
         */
        bool isCreate() const;

        /*!
         * @brief Indicates whether the event is an "update" event.
         *
         * This is equivalent to comparing Event<>::mt() against
         * DotsMt::update.
         *
         * Note that this is always false for uncached types.
         *
         * @return true If the event is an "update" event (i.e. Event<>::mt()
         * == DotsMt::update is true).
         * @return false Else.
         */
        bool isUpdate() const;

        /*!
         * @brief Indicates whether the event is a "remove" event.
         *
         * This is equivalent to comparing Event<>::mt() against
         * DotsMt::remove.
         *
         * Note that this is always false for uncached types.
         *
         * @return true If the event is an "remove" event (i.e. Event<>::mt()
         * == DotsMt::remove is true).
         * @return false Else.
         */
        bool isRemove() const;

        /*!
         * @brief Indicates whether the event was triggered by "myself".
         *
         * This will indicate whether the transmission that triggered this
         * event was originally transmitted due to a publish from the current
         * transceiver (i.e. the one that is processing this event).
         *
         * @return true If the event was triggered from the current
         * transceiver.
         * @return false Else.
         */
        bool isFromMyself() const;

        /*!
         * @brief Get the transmitted properties.
         *
         * This will return the published properties contained in the
         * transmission.
         *
         * Note that if the publish sought to invalidate certain properties of
         * an instance, these might refer to properties that are not valid in
         * Event<>::transmitted().
         *
         * @return property_set_t The property set contained in the
         * transmission that triggered this event.
         */
        property_set_t newProperties() const { return header().attributes; }

        /*!
         * @brief Get the updated properties.
         *
         * This will return the valid properties of Event<>::updated() that
         * were affected by the transmission.
         *
         * Note that this does not include properties that were invalidated by
         * the transmission.
         *
         * @return property_set_t 
         */
        property_set_t updatedProperties() const { return newProperties() ^ updated()._validProperties(); }

        /*!
         * @brief Safely cast the Event<> to the explicitly typed version.
         *
         * @tparam T The DOTS type to safely cast to.
         *
         * @return const Event<T>& A reference to the event casted to the
         * explicitly typed version.
         *
         * @exception std::runtime_error Thrown if @p T does not match the type
         * of the Container.
         */
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

        /*!
         * @brief Safely cast the Event<> to the explicitly typed version.
         *
         * @tparam T The DOTS type to safely cast to.
         *
         * @return Event<T>& A reference to the event casted to the explicitly
         * typed version.
         *
         * @exception std::runtime_error Thrown if @p T does not match the type
         * of the Container.
         */
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
        DotsMt m_mt;
    };

    /*!
     * @brief Explicitly typed version of Event<>.
     *
     * This is an explicitly typed version of Event<>. It inherits all
     * capabilities from the Event<> specialization, but also offers
     * various functions in a typed variant.
     *
     * Instances of this class contain information about a specific DOTS
     * event. Events occur when incoming transmissions are dispatched (i.e.
     * processed) by a Transceiver to registered event handlers (i.e.
     * subscribers) of a particular DOTS struct type.
     *
     * If the struct type is cached, events refer to the effective
     * operation after the local Container instance was updated. Uncached
     * types, which do not have a container, will always be presented as
     * DotsMt::create events to the user.
     *
     * @tparam T The DOTS struct type of the event.
     *
     * @remark Event<T> instances are read-only views of the event data and
     * are only valid during dispatch of the event. They are therefore
     * neither copyable nor movable.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create or manage Event<T> objects themselves.
     */
    template<typename T>
    struct Event : Event<type::Struct>
    {
        static_assert(std::is_base_of_v<type::Struct, T>);

        /*!
         * @brief Construct a new Event<T> object.
         *
         * Note that the Event<T> will store only references to most of the
         * arguments. The referred objects must therefore stay valid until the
         * event has been processed.
         *
         * @param header The header contained in the transmission that
         * triggered the event. If the transmission was originally transmitted
         * from the current transceiver (i.e. the one that is processing this
         * event), the @p isFromMyself flag has to be set to true.
         *
         * @param transmitted The instance contained in the transmission that
         * triggered the event.
         *
         * @param updated The updated instance of the event. For cached types,
         * this refers to the instance in the local Container. Otherwise it
         * must be the same instance as @p transmitted .
         *
         * @param cloneInfo The clone information of the event. For cached
         * types, this refers to the clone info in the local Container.
         *
         * @param mt The operation (i.e. "type" or "category") of the event. If
         * no operation is given, the last operation of @p cloneInfo will be
         * used.
         */
        Event(const DotsHeader& header, const T& transmitted, const T& updated, const DotsCloneInformation& cloneInfo, std::optional<DotsMt> mt = std::nullopt) :
            Event<type::Struct>(header, transmitted, updated, cloneInfo, mt)
        {
            /* do nothing */
        }
        Event(const Event& other) = delete;
        Event(Event&& other) = delete;
        ~Event() = default;

        Event& operator = (const Event& rhs) = delete;
        Event& operator = (Event&& rhs) = delete;

        /*!
         * @brief Get the updated instance.
         *
         * Using this operator is short hand for Event<T>::updated().
         *
         * For cached types, the returned instance refers to the instance in
         * the local Container.
         *
         * For uncached types, this is equivalent to Event<T>::transmitted().
         *
         * @return const T& A reference to the updated instance.
         */
        const T& operator () () const
        {
            return static_cast<const T&>(Event<type::Struct>::operator()());
        }

        /*!
         * @brief Get the transmitted instance.
         *
         * This is the unaltered instance as it was contained in the
         * transmission that triggered this event.
         *
         * @return const T& A reference to the transmitted instance.
         */
        const T& transmitted() const
        {
            return static_cast<const T&>(Event<type::Struct>::transmitted());
        }

        /*!
         * @brief Get the updated instance.
         *
         * For cached types, the returned instance refers to the instance in
         * the local Container (i.e. the "local clone").
         *
         * For uncached types, this is equivalent to Event<>::transmitted().
         *
         * @return const T& A reference to the updated instance.
         */
        const T& updated() const
        {
            return static_cast<const T&>(Event<type::Struct>::updated());
        }
    };
}
