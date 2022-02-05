#include <dots/io/Io.h>

namespace dots::io
{
    static boost::asio::io_context IoContext;

    boost::asio::io_context& global_io_context()
    {
        return IoContext;
    }

    boost::asio::execution_context& global_execution_context()
    {
        return global_io_context();
    }

    boost::asio::executor global_executor()
    {
        return global_io_context().get_executor();
    }
}
