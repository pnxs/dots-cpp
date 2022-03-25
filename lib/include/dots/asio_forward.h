#pragma once

namespace boost::asio
{
    class io_context;
    class execution_context;
    class executor;

    namespace ip
    {
        template <typename InternetProtocol>
        class basic_endpoint;

        class tcp;
    }
}

namespace dots
{
    namespace asio = boost::asio;
}
