#include "TcpAcceptor.h"
#include <dots/eventloop/IoService.h>

namespace dots
{

TcpAcceptor::TcpAcceptor(): TcpAcceptorBase(ioService())
{
}

TcpAcceptor::TcpAcceptor(IoService& ioService): TcpAcceptorBase(ioService)
{
}


}