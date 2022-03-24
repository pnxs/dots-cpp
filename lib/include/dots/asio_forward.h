#pragma once

namespace boost::asio
{
    class io_context;
    class execution_context;
    class executor;

    template <typename Executor>
    class basic_signal_set;

    typedef basic_signal_set<executor> signal_set;

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
