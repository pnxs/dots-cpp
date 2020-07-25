#include "Listener.h"

namespace dots::io
{
    void Listener::asyncAccept(accept_handler_t&& acceptHandler, error_handler_t&& errorHandler)
    {
        if (m_asyncAcceptActive)
        {
            throw std::logic_error{ "only one async accept can be active at the same time" };
        }

        m_asyncAcceptActive = true;
        m_acceptHandler = std::move(acceptHandler);
        m_errorHandler = std::move(errorHandler);
        asyncAcceptImpl();
    }

    void Listener::processAccept(channel_ptr_t channel)
    {
        if (m_acceptHandler(*this, std::move(channel)))
        {
            asyncAcceptImpl();
        }
        else
        {
            m_asyncAcceptActive = false;
            m_acceptHandler = nullptr;
            m_errorHandler = nullptr;
        }
    }

    void Listener::processError(const std::exception_ptr& e)
    {
        if (m_errorHandler != nullptr)
        {
            m_errorHandler(*this, e);
        }
    }

    void Listener::processError(const std::string& what)
    {
        processError(std::make_exception_ptr(std::runtime_error{ what }));
    }

    void Listener::verifyErrorCode(const std::error_code& errorCode)
    {
        if (errorCode)
        {
            throw std::system_error{ errorCode };
        }
    }
}