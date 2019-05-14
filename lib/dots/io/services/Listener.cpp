#include "Listener.h"

namespace dots
{
    void Listener::asyncAccept(accept_handler_t&& acceptHandler)
    {
        if (m_asyncAcceptActive)
        {
            throw std::logic_error{ "only one async accept can be active at the same time" };
        }

        m_asyncAcceptActive = true;
        m_acceptHandler = std::move(acceptHandler);
        asyncAcceptImpl();
    }

    void Listener::processAccept(channel_ptr_t channel)
    {
        if (m_acceptHandler(std::move(channel)))
        {
            asyncAcceptImpl();
        }
        else
        {
            m_asyncAcceptActive = false;
            m_acceptHandler = nullptr;
        }
    }
}