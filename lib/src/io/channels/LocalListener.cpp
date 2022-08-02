// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
