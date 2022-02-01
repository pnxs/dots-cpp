#pragma once
#include <memory>
#include <optional>
#include <dots/tools/Handler.h>
#include <dots/io/Channel.h>

namespace dots::io
{
    struct Listener
    {
        using accept_handler_t = tools::Handler<bool(Listener&, channel_ptr_t)>;
        using error_handler_t = tools::Handler<void(Listener&, std::exception_ptr)>;

        Listener() = default;
        Listener(const Listener& other) = delete;
        Listener(Listener&& other) = delete;
        virtual ~Listener() = default;

        Listener& operator = (const Listener& rhs) = delete;
        Listener& operator = (Listener&& rhs) = delete;

        void asyncAccept(accept_handler_t acceptHandler, error_handler_t errorHandler);

    protected:

        virtual void asyncAcceptImpl() = 0;
        void processAccept(channel_ptr_t channel);
        void processError(std::exception_ptr ePtr);
        void processError(const std::string& what);
        void verifyErrorCode(std::error_code errorCode);

    private:

        bool m_asyncAcceptActive = false;
        std::optional<accept_handler_t> m_acceptHandler;
        std::optional<error_handler_t> m_errorHandler;
    };

    using listener_ptr_t = std::unique_ptr<Listener>;
}
