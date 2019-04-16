#include "TcpResolver.h"

namespace dots
{

TcpResolver::TcpResolver(): base(AsioEventLoop::Instance().ioService())
{
}

TcpResolver::TcpResolver(boost::asio::io_service&ioService): base(ioService)
{
}

TcpResolver::~TcpResolver()
{
}

}