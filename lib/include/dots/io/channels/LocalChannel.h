#pragma once
#include <dots/asio.h>
#include <dots/io/Channel.h>

namespace dots::io
{
    struct LocalListener;

    struct LocalChannel : Channel
    {
        LocalChannel(key_t key, asio::io_context& ioContext);
        LocalChannel(key_t key, asio::io_context& ioContext, LocalListener& peer);
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

        std::reference_wrapper<asio::io_context> m_ioContext;
        std::weak_ptr<LocalChannel> m_peer;
    };
}
