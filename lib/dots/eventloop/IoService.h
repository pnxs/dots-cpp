#pragma once

#include <boost/asio/io_service.hpp>

namespace dots {

class IoService: public boost::asio::io_service
{
public:
    IoService() = default;
    //~IoService() = default;
};

IoService& ioService();

}