#include <dots/io/services/Channel.h>
#include <dots/io/Registry.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>
#include <DotsMsgError.dots.h>

namespace dots
{
    Channel::Channel(key_t key) :
        shared_ptr_only(key),
        m_initialized(false),
        m_registry(nullptr)
    {
        /* do nothing */
    }

    void Channel::init(io::Registry& registry)
    {
        if (m_initialized)
        {
            throw std::logic_error{ "channel has already been initialized" };
        }

        m_registry = &registry;
        m_initialized = true;
    }

    void Channel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
    {
        verifyInitialized();

        if (receiveHandler == nullptr || errorHandler == nullptr)
        {
            throw std::logic_error{ "both a receive and an error handler must be set" };
        }
        
        m_receiveHandler = std::move(receiveHandler);
        m_errorHandler = std::move(errorHandler);
        asyncReceiveImpl();
    }

    void Channel::transmit(const type::Struct& instance)
    {
        transmit(DotsTransportHeader{
            DotsTransportHeader::destinationGroup_i{ instance._descriptor().name() },
            DotsTransportHeader::dotsHeader_i{
                DotsHeader::typeName_i{ instance._descriptor().name() },
				DotsHeader::attributes_i{ instance._validProperties() }
            }
        }, instance);
    }

    void Channel::transmit(const DotsHeader& dotsHeader, const type::Struct& instance)
    {
        transmit(DotsTransportHeader{ 
            DotsTransportHeader::destinationGroup_i{ dotsHeader.typeName },
            DotsTransportHeader::dotsHeader_i{ dotsHeader }
        }, instance);
    }

    void Channel::transmit(const DotsTransportHeader& header, const type::Struct& instance)
    {
        verifyInitialized();
        transmitImpl(header, instance);
    }

    void Channel::transmit(const DotsTransportHeader& header, const Transmission& transmission)
    {
        verifyInitialized();
        transmitImpl(header, transmission);
    }

    const io::Registry& Channel::registry() const
    {
        verifyInitialized();
	    return *m_registry;
    }
	
    io::Registry& Channel::registry()
    {
        verifyInitialized();
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

    void Channel::verifyInitialized() const
    {
        if (!m_initialized)
        {
            throw std::logic_error{ "channel has not been initialized" };
        }
    }
}