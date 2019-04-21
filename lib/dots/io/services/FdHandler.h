#pragma once
#include "dots/cpp_config.h"
#include <asio.hpp>

namespace dots {

class FdHandler
{
    asio::posix::stream_descriptor m_sd;
    function<void()> m_handler;

    /**
     * Calls registered handler-function and starts a new read.
     */
    void on_read_finished()
    {
        m_handler();
        start_read();
    }

public:
    FdHandler(asio::io_context& ioContext, int fd, function<void()> handler)
            : m_sd(ioContext, fd), m_handler(handler)
    {
        start_read();
    }

    void start_read()
    {
        m_sd.async_read_some(
                asio::null_buffers(),
                std::bind(&FdHandler::on_read_finished, this));
    }

};

}