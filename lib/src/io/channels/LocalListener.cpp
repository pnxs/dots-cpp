#include <dots/io/channels/LocalListener.h>

namespace dots::io
{
    LocalListener::LocalListener(asio::io_context& ioContext) :
        m_ioContext{ std::ref(ioContext) }
    {
        /* do nothing */
    }

    void LocalListener::accept(LocalChannel& peer)
    {
        asio::post(m_ioContext.get(), [&]
        {
            auto channel = make_channel<LocalChannel>(static_cast<asio::io_context&>(m_ioContext));
            peer.link(*channel);
            channel->link(peer);
            processAccept(std::move(channel));
        });
    }

    void LocalListener::asyncAcceptImpl()
    {
        /* do nothing */
    }
}
