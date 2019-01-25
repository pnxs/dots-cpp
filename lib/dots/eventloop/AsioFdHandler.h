#pragma once
#include "dots/cpp_config.h"
#include <boost/asio.hpp>

namespace dots {

class AsioFdHandler
{
    boost::asio::posix::stream_descriptor m_sd;
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
    AsioFdHandler(boost::asio::io_service &ioservice, int fd, function<void()> handler)
            : m_sd(ioservice, fd), m_handler(handler)
    {
        start_read();
    }

    void start_read()
    {
        m_sd.async_read_some(
                boost::asio::null_buffers(),
                std::bind(&AsioFdHandler::on_read_finished, this));
    }

};

}