#include <dots/io/Io.h>

namespace dots::io
{
    static asio::io_context IoContext;

    asio::io_context& global_io_context()
    {
        return IoContext;
    }

    asio::execution_context& global_execution_context()
    {
        return global_io_context();
    }

    asio::executor global_executor()
    {
        return global_io_context().get_executor();
    }
}
