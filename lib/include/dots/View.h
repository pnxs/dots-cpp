// SPDX-License-Identifier: LGPL-3.0-only
#pragma once
#include <cstdint>
#include <utility>
#include <vector>
#include <dots/Container.h>
#include <dots/Event.h>
#include <dots/FilterBuilder.h>
#include <dots/Subscription.h>
#include <dots/tools/Handler.h>
#include <DotsFilter.dots.h>

namespace dots
{
    struct GuestTransceiver;

    namespace io
    {
        struct Transmission;
    }

    namespace details
    {
        // Type-erased base of View<T>. The GuestTransceiver demux table holds
        // ViewBase*; routing dispatches a Transmission whose header carries the
        // matching subscriptionId to view->_dispatch(transmission).
        struct ViewBase
        {
            ViewBase() = default;
            ViewBase(const ViewBase&) = delete;
            ViewBase(ViewBase&&) = delete;
            virtual ~ViewBase() = default;
            ViewBase& operator=(const ViewBase&) = delete;
            ViewBase& operator=(ViewBase&&) = delete;

            uint32_t _subscriptionId() const { return m_subId; }
            const DotsFilter& _filter() const { return m_filter; }

            virtual void _dispatch(const io::Transmission& transmission) = 0;

        protected:
            uint32_t m_subId{};
            DotsFilter m_filter{};
        };
    }

    /*!
     * @brief A filtered subscription view onto a DOTS type.
     *
     * A View<T> opens its own wire-level filtered subscription to the broker,
     * carries a per-View container holding only the matching (and projected)
     * instances, and routes events to handlers attached via subscribe().
     *
     * Construct via dots::view<T>(filter) — the free function takes care of
     * the global transceiver and the capability check.
     *
     * The View is non-copyable and non-movable; hold it as a local variable or
     * a member. Returning one from a factory function is fine thanks to C++17
     * mandatory copy elision (the prvalue is constructed in place at the call
     * site).
     */
    template <typename T>
    struct View : details::ViewBase
    {
        using value_t      = T;
        using event_t      = Event<T>;
        using event_base_t = Event<type::Struct>;
        using handler_t    = tools::Handler<void(const event_t&)>;

        View(GuestTransceiver& transceiver, DotsFilter filter);
        ~View() override;

        /*!
         * @brief Attach a handler that fires for events delivered to this view.
         *
         * The handler is invoked synchronously for each instance already in
         * this view's container (i.e. instances that have arrived during
         * preload or earlier publishes), then asynchronously for subsequent
         * events.
         *
         * @return Subscription RAII handle. When destroyed, the handler is
         * removed from the view.
         */
        Subscription subscribe(handler_t handler);

        const Container<T>& container() const & { return m_container; }
        size_t size() const { return m_container.size(); }
        uint32_t subscriptionId() const { return m_subId; }

        void _dispatch(const io::Transmission& transmission) override;

    private:
        using handler_id_t = uint32_t;
        GuestTransceiver* m_tx{};
        Container<T> m_container{};
        std::vector<std::pair<handler_id_t, handler_t>> m_handlers{};
        handler_id_t m_nextHandlerId{ 1 };
        // Handlers may subscribe or unsubscribe re-entrantly from within a
        // dispatch: removals are deferred while m_invokeDepth > 0 and drained
        // when the dispatch unwinds (mirrors the core Dispatcher's behavior).
        uint32_t m_invokeDepth{ 0 };
        std::vector<handler_id_t> m_removeIds{};

        void invokeHandlers(const event_base_t& event);
        void eraseHandler(handler_id_t id);
        void drainRemovedHandlers();
        bool isMarkedForRemoval(handler_id_t id) const;
    };

    /*!
     * @brief Open a filtered subscription on the global transceiver.
     *
     * @param filter Builder produced via dots::filter::predicate(...) and/or
     *               dots::filter::project(...).
     *
     * @exception std::runtime_error If the broker did not advertise the
     *            filteredSubscriptions capability in its Hello.
     */
    template <typename T>
    View<T> view(filter::Filter f);

    template <typename T>
    View<T> view(DotsFilter f);

    template <typename T>
    inline View<T> view(filter::Predicate p)
    {
        return view<T>(filter::predicate(std::move(p)));
    }
}

#include <dots/View.inl>
