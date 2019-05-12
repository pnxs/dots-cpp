#include "Channel.h"

namespace dots
{
    void Channel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
    {
        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        asyncReceiveImpl();
    }

    void Channel::transmit(const DotsTransportHeader& header, const type::Struct& instance)
    {
        transmitImpl(header, instance);
    }

    void Channel::transmit(const DotsTransportHeader& header, const Transmission& transmission)
    {
        transmitImpl(header, transmission);
    }

    void Channel::transmitImpl(const DotsTransportHeader& header, const Transmission& transmission)
    {
        transmitImpl(header, transmission.instance());
    }

    void Channel::processReceive(const DotsTransportHeader& haeder, Transmission&& transmission)
    {
        if (m_receiveHandler(haeder, std::move(transmission)))
        {
            asyncReceiveImpl();
        }
    }
	
    void Channel::processError(int ec)
    {
        if (m_errorHandler != nullptr)
        {
            m_errorHandler(ec);
        }        
    }
}