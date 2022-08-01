// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
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
