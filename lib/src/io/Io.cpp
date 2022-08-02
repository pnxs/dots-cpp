// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/Io.h>

namespace dots::io
{
    static asio::io_context IoContext;

    asio::io_context& global_io_context()
    {
        return IoContext;
    }

    asio::execution_context& global_execution_context()
    {
        return global_io_context();
    }

    asio::executor global_executor()
    {
        return global_io_context().get_executor();
    }
}
