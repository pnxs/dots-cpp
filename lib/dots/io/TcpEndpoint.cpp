#include "TcpEndpoint.h"

namespace dots {

string TcpEndpoint::toString() const
{
    return address().to_string() + ":" + std::to_string(port());
}

}