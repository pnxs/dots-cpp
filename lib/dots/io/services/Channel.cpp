#include "Channel.h"

namespace dots
{
    void Channel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
    {
        if (m_asyncReceiveActive)
        {
            throw std::logic_error{ "only one async receive can be active at the same time" };
        }

        m_asyncReceiveActive = true;
        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        asyncReceiveImpl();
    }

    void Channel::transmit(const DotsTransportHeader& header, const type::NewStruct& instance)
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

    void Channel::processReceive(const DotsTransportHeader& header, Transmission&& transmission)
    {
        if (m_receiveHandler(header, std::move(transmission)))
        {
            asyncReceiveImpl();
        }
        else
        {
            m_asyncReceiveActive = false;
            m_receiveHandler = nullptr;
            m_errorHandler = nullptr;
        }
    }
	
    void Channel::processError(const std::exception& e)
    {
        if (m_errorHandler != nullptr)
        {
            m_errorHandler(e);
        }        
    }

    void Channel::processError(const std::string& what)
    {
        processError(std::runtime_error{ what });
    }

    void Channel::verifyErrorCode(const std::error_code& errorCode)
    {
        if (errorCode)
        {
            throw std::system_error{ errorCode };
        }
    }
}