#pragma once
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
#include <dots/tools/Handler.h>

namespace dots::io::posix
{
    struct FdHandler
    {
        FdHandler(asio::io_context& ioContext, int fd, tools::Handler<void()> handler) :
            m_sd(ioContext, fd),
            m_handler(std::move(handler))
        {
            start_read();
        }

        void start_read()
        {
            m_sd.async_read_some(asio::null_buffers(), std::bind(&FdHandler::on_read_finished, this));
        }

    private:

        /**
         * Calls registered handler-function and starts a new read.
         */
        void on_read_finished()
        {
            m_handler();
            start_read();
        }

        asio::posix::stream_descriptor m_sd;
        tools::Handler<void()> m_handler;
    };
}
#else
#error #error "POSIX stream descriptors are not available on this platform"
#endif
