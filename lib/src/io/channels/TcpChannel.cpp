// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#include <dots/io/channels/TcpChannel.h>

namespace dots::io::details
{
    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericTcpChannel<Serializer, TransmissionFormat>::GenericTcpChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint) :
        GenericTcpChannel(key, ioContext, endpoint.host(), endpoint.port().empty() ? DefaultPort : endpoint.port())
    {
        /* do nothing */
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericTcpChannel<Serializer, TransmissionFormat>::GenericTcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port) :
        GenericTcpChannel(key, stream_t{ ioContext }, nullptr)
    {
        auto endpoints = m_resolver.resolve(asio::ip::tcp::socket::protocol_type::v4(), host, port, asio::ip::resolver_query_base::numeric_service);

        for (const auto& endpoint: endpoints)
        {
            try
            {
                stream().connect(endpoint);
                setDefaultSocketOptions();
                initEndpoints(Endpoint{ stream().local_endpoint() }, Endpoint{ stream().remote_endpoint() });

                return;
            }
            catch (const std::exception&/* e*/)
            {
                /* do nothing */
            }
        }

        throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericTcpChannel<Serializer, TransmissionFormat>::GenericTcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port, std::function<void(const boost::system::error_code& error)> onConnect) :
        GenericTcpChannel(key, asio::ip::tcp::socket{ ioContext }, nullptr)
    {
        asyncResolveEndpoint(host, port, [this, onConnect{ std::move(onConnect) }](auto& error, auto endpoint) {
            if (error)
            {
                onConnect(error);
            }

            stream().async_connect(*endpoint, [this, endpoint, onConnect{ std::move(onConnect) }](const boost::system::error_code& error) {
                if (error)
                {
                    onConnect(error);
                    //throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
                }
                else
                {
                    setDefaultSocketOptions();
                    initEndpoints(Endpoint{ stream().local_endpoint() }, Endpoint{ stream().remote_endpoint() });
                    onConnect(error);
                }
            });
        });
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    GenericTcpChannel<Serializer, TransmissionFormat>::GenericTcpChannel(key_t key, asio::ip::tcp::socket&& socket_, payload_cache_t* payloadCache) :
        base_t(key, std::move(socket_), payloadCache),
        m_resolver( stream().get_executor())
    {
        if (stream().is_open())
        {
            initEndpoints(Endpoint{ stream().local_endpoint() }, Endpoint{ stream().remote_endpoint() });
        }
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    void GenericTcpChannel<Serializer, TransmissionFormat>::setDefaultSocketOptions()
    {
        stream().set_option(asio::ip::tcp::no_delay(true));
        stream().set_option(asio::ip::tcp::socket::keep_alive(true));
        stream().set_option(asio::socket_base::linger(true, 10));
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    void GenericTcpChannel<Serializer, TransmissionFormat>::asyncResolveEndpoint(std::string_view host, std::string_view port, resolve_handler_t handler)
    {
        m_resolver.async_resolve(host, port, asio::ip::resolver_query_base::numeric_service, [handler{ std::move(handler) }](const boost::system::error_code& error, auto iter) {
            if (error)
            {
                handler(error, {});
                return;
            }

            decltype(iter) iterEnd;

            for (; iter != iterEnd; ++iter)
            {
                const auto& address = iter->endpoint().address();

                if (address.is_v4() || address.is_v6())
                {
                    handler(error, iter->endpoint());
                    return;
                }
            }

            handler(boost::system::error_code(boost::system::errc::io_error, boost::system::generic_category()), {});
        });
    }

    template <typename Serializer, TransmissionFormat TransmissionFormat>
    void GenericTcpChannel<Serializer, TransmissionFormat>::verifyErrorCode(const boost::system::error_code& ec)
    {
        if (ec == asio::error::misc_errors::eof || ec == asio::error::basic_errors::bad_descriptor)
        {
            throw std::runtime_error{ "channel was closed unexpectedly: " + ec.message() };
        }
        else if (ec)
        {
            throw std::system_error{ ec };
        }
    }

    template struct GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    template struct GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v2>;
}
