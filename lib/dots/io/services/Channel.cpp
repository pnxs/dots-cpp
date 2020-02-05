#include <dots/io/services/Channel.h>
#include <dots/io/Registry.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMsgError.dots.h>

namespace dots
{
    void Channel::asyncReceive(io::Registry& registry, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
    {
        if (m_asyncReceiveActive)
        {
            throw std::logic_error{ "only one async receive can be active at the same time" };
        }

        if (receiveHandler == nullptr || errorHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and an error handler must be set" };
        }

        m_asyncReceiveActive = true;
    	m_registry = &registry;
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

    const io::Registry& Channel::registry() const
    {
	    return *m_registry;
    }
	
    io::Registry& Channel::registry()
    {
	    return *m_registry;
    }

    void Channel::transmitImpl(const DotsTransportHeader& header, const Transmission& transmission)
    {
        transmitImpl(header, transmission.instance());
    }

    void Channel::processReceive(const DotsTransportHeader& header, Transmission&& transmission) noexcept
    {
        try
        {
            if (m_receiveHandler(header, std::move(transmission)))
            {
                asyncReceiveImpl();
            }
            else
            {
                m_asyncReceiveActive = false;
        	    m_registry = nullptr;
                m_receiveHandler = nullptr;
                m_errorHandler = nullptr;
            }
        }
        catch (...)
        {
            processError(std::current_exception());
        }
    }
	
    void Channel::processError(const std::exception_ptr& e)
    {
        try
        {
            m_errorHandler(e);
        }
        catch (const std::exception& e)
        {
            throw std::logic_error{ std::string{ "exception in channel error handler -> " } + e.what() };
        }       
    }

    void Channel::processError(const std::string& what)
    {
        processError(std::make_exception_ptr(std::runtime_error{ what }));
    }

    void Channel::verifyErrorCode(const std::error_code& errorCode)
    {
        if (errorCode)
        {
            throw std::system_error{ errorCode };
        }
    }
}