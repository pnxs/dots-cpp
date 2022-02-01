#include <dots/io/auth/AuthManager.h>

namespace dots::io
{
    AuthManager::AuthManager(HostTransceiver& transceiver) :
        m_transceiver(&transceiver)
    {
        /* do nothing */
    }

    const HostTransceiver& AuthManager::transceiver() const
    {
        return *m_transceiver;
    }

    HostTransceiver& AuthManager::transceiver()
    {
        return *m_transceiver;
    }
}
