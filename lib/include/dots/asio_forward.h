// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

namespace boost::asio
{
    class io_context;
    class execution_context;
    class executor;

    namespace ip
    {
        template <typename InternetProtocol>
        class basic_endpoint;

        class tcp;
    }
}

namespace dots
{
    namespace asio = boost::asio;
}
