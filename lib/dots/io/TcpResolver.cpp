#include "TcpResolver.h"

namespace dots
{

TcpResolver::TcpResolver(): base(ioService())
{
}

TcpResolver::TcpResolver(IoService &ioService): base(ioService)
{
}

TcpResolver::~TcpResolver()
{
}

}