#pragma once
#include <boost/asio.hpp>
#include <dots/io/Listener.h>
#include <dots/io/Channel.h>

namespace dots::io
{
    struct ChannelService : boost::asio::execution_context::service
    {
        using key_type = ChannelService;

        explicit ChannelService(boost::asio::execution_context& executionContext) :
            boost::asio::execution_context::service(executionContext)
        {
            /* do nothing */
        }
        ChannelService(const ChannelService& other) = delete;
        ChannelService(ChannelService&& other) noexcept(false) = delete;
        ~ChannelService() = default;

        ChannelService& operator = (const ChannelService& rhs) = delete;
        ChannelService& operator = (ChannelService&& rhs) noexcept(false) = delete;

        template <typename TListener, typename... Args>
        listener_ptr_t makeListener(Args&&... args)
        {
            return std::make_unique<TListener>(static_cast<boost::asio::io_context&>(context()), std::forward<Args>(args)...);
        }

        template <typename TChannel, typename... Args>
        channel_ptr_t makeChannel(Args&&... args)
        {
            auto channel = make_channel<TChannel>(static_cast<boost::asio::io_context&>(context()), std::forward<Args>(args)...);
            return channel;
        }

    private:

        void shutdown() noexcept override
        {
            /* do nothing */
        }
    };
}