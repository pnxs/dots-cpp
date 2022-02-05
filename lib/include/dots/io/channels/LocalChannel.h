#pragma once
#include <boost/asio.hpp>
#include <dots/io/Channel.h>

namespace dots::io
{
    struct LocalListener;

    struct LocalChannel : Channel
    {
        LocalChannel(Channel::key_t key, boost::asio::io_context& ioContext);
        LocalChannel(Channel::key_t key, boost::asio::io_context& ioContext, LocalListener& peer);
        LocalChannel(const LocalChannel& other) = delete;
        LocalChannel(LocalChannel&& other) = delete;
        ~LocalChannel() override = default;

        LocalChannel& operator = (const LocalChannel& rhs) = delete;
        LocalChannel& operator = (LocalChannel&& rhs) = delete;

        void link(LocalChannel& other);

    protected:

        void asyncReceiveImpl() override;
        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override;

    private:

        std::reference_wrapper<boost::asio::io_context> m_ioContext;
        std::weak_ptr<LocalChannel> m_peer;
    };
}
