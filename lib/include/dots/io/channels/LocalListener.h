#pragma once
#include <functional>
#include <optional>
#include <dots/asio.h>
#include <dots/io/Listener.h>
#include <dots/io/channels/LocalChannel.h>

namespace dots::io
{
    struct LocalListener : Listener
    {
        LocalListener(asio::io_context& ioContext);
        LocalListener(const LocalListener& other) = delete;
        LocalListener(LocalListener&& other) = delete;
        ~LocalListener() override = default;

        LocalListener& operator = (const LocalListener& rhs) = delete;
        LocalListener& operator = (LocalListener&& rhs) = delete;

        void accept(LocalChannel& peer);

    protected:

        void asyncAcceptImpl() override;

    private:

        std::reference_wrapper<asio::io_context> m_ioContext;
    };
}
