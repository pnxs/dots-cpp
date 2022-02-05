#pragma once
#include <functional>
#include <dots/asio.h>

namespace dots::io
{
    asio::io_context& global_io_context();
    asio::execution_context& global_execution_context();
    asio::executor global_executor();

    template <typename Service>
    Service& global_service()
    {
        return asio::use_service<Service>(global_execution_context());
    }

    template <typename Handler>
    auto post_and_promise_result(asio::io_context& ioContext, Handler&& handler)
    {
        using handler_t = std::decay_t<Handler>;

        constexpr bool IsTriviallyInvocable = std::is_invocable_v<handler_t>;
        static_assert(IsTriviallyInvocable, "Handler has to be trivially invocable (i.e. be callable without arguments)");

        if constexpr (IsTriviallyInvocable)
        {
            using return_t = std::invoke_result_t<handler_t>;

            std::promise<return_t> promise;
            std::future<return_t> future = promise.get_future();

            asio::post(ioContext, [promise{ std::move(promise) }, handler{ std::forward<Handler>(handler) }]() mutable 
            {
                if constexpr (std::is_same_v<return_t, void>)
                {
                    std::invoke(handler);
                    promise.set_value();
                }
                else
                {
                    promise.set_value(std::invoke(handler));
                }
            });

            return future;
        }
        else
        {
            return std::declval<std::future<void>>();
        }
    }

    template <typename Handler>
    auto post_and_promise_result(Handler&& handler)
    {
        return post_and_promise_result(global_io_context(), std::forward<Handler>(handler));
    }
}
