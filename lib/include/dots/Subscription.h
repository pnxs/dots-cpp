#pragma once
#include <functional>

namespace dots
{
    /*!
     * @class Subscription Subscription.h <dots/Subscription.h>
     *
     * @brief Scoped resource for active DOTS subscriptions.
     *
     * An object of this class is a RAII-style resource that represents an
     * active DOTS subscription (e.g. created by dots::subscribe()).
     *
     * Subscription objects are movable but cannot be copied.
     *
     * Technically, a Subscription accepts an unsubscribe handler upon
     * construction that is invoked once the Subscription goes out of
     * scope.
     *
     * @attention Outside of advanced use cases, a regular user is never
     * required to create Subscription objects themselves. Instead, they
     * are always obtained by calling corresponding subscribe functions of
     * a dots::Transceiver.
     */
    struct [[nodiscard]] Subscription
    {
        using unsubscribe_handler_t = std::function<void()>;

        /*!
         * @brief Construct a new Subscription object.
         *
         * @param handler The unsubscribe handler that will be invoked when the
         * Subscription goes out of scope (see ~Subscription()).
         */
        Subscription(unsubscribe_handler_t handler);

        Subscription(const Subscription& other) = delete;

        /*!
         * @brief Construct a new Subscription object by moving an existing
         * one.
         *
         * This will create a new Subscription object by transferring ownership
         * of an active DOTS subscription from an existing Subscription object.
         *
         * @param other The Subscription object whose active DOTS subscription
         * to transfer. Note that after the move @p other is empty and can
         * safely be destroyed without invoking the unsubscribe handler.
         */
        Subscription(Subscription&& other) noexcept;

        /*!
         * @brief Destroy the Subscription object.
         *
         * When the Subscription object is destroyed and is managing an active
         * subscription (i.e. is not empty), the subscription will be cancelled
         * by invoking the unsubscribe handler (see unsubscribe()).
         */
        ~Subscription();

        Subscription& operator = (const Subscription& rhs) = delete;

        /*!
         * @brief Assign an active subscription to this Subscription object by
         * moving an existing one.
         *
         * This will transfer ownership of an active DOTS subscription from an
         * existing Subscription object to this one.
         *
         * @attention If this Subscription object already manages an active
         * subscription (i.e. is not empty), the subscription will be cancelled
         * before the assignment (see unsubscribe()).
         *
         * @param rhs The Subscription object whose active DOTS subscription to
         * transfer. Note that after the move @p rhs is empty and can safely be
         * destroyed without invoking the unsubscribe handler.
         *
         * @return Subscription& A reference to this Subscription.
         */
        Subscription& operator = (Subscription&& rhs) noexcept;

        /*!
         * @brief Cancel the active subscription by invoking the unsubscribe
         * handler.
         *
         * This will leave the Subscription object in an empty state if it is
         * managing an active subscription before the call.
         *
         * Note that this will have no effect if the Subscription object is
         * already empty when the function is called.
         *
         * @remark This function is called by ~Subscription() when the
         * Subscription is destroyed.
         */
        void unsubscribe();

        /*!
         * @brief Release management of the active subscription.
         *
         * Calling this function will decouple the active subscription from the
         * Subscription object's lifetime, without invoking the unsubscribe
         * handler. As a result, this Subscription object will be empty when
         * the function returns.
         *
         * Note that this will have no effect if the Subscription object is
         * already empty when the function is called.
         *
         * @warning Calling this function will make it impossible to manually
         * cancel the active subscription.
         *
         * @remark This function is intended to cover simple use cases where
         * the durations of subscriptions are bound to an application's
         * lifetime and subscription management is not required.
         */
        void discard();

    private:

        unsubscribe_handler_t m_handler;
    };
}