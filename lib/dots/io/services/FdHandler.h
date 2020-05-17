#pragma once
#include <boost/asio.hpp>

namespace dots::io {

class FdHandler
{
    boost::asio::posix::stream_descriptor m_sd;
    std::function<void()> m_handler;

    /**
     * Calls registered handler-function and starts a new read.
     */
    void on_read_finished()
    {
        m_handler();
        start_read();
    }

public:
    FdHandler(boost::asio::io_context& ioContext, int fd, std::function<void()> handler)
            : m_sd(ioContext, fd), m_handler(handler)
    {
        start_read();
    }

    void start_read()
    {
        m_sd.async_read_some(
                boost::asio::null_buffers(),
                std::bind(&FdHandler::on_read_finished, this));
    }

};

}