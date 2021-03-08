#include <dots/io/channels/TcpChannel.h>
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/CborNativeSerialization.h>
#include <DotsClient.dots.h>
#include <DotsDescriptorRequest.dots.h>

namespace dots::io
{
    TcpChannel::TcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port) :
        TcpChannel(key, boost::asio::ip::tcp::socket{ ioContext })
    {
        auto endpoints = m_resolver.resolve(boost::asio::ip::tcp::socket::protocol_type::v4(), host, port, boost::asio::ip::resolver_query_base::numeric_service);

        for (const auto& endpoint: endpoints)
        {
            try
            {
                m_socket.connect(endpoint);
                setDefaultSocketOptions();
                determineEndpoints();

                return;
            }
            catch (const std::exception&/* e*/)
            {
                /* do nothing */
            }
        }

        throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
    }

    TcpChannel::TcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port, std::function<void(const boost::system::error_code& error)> onConnect) :
        TcpChannel(key, boost::asio::ip::tcp::socket{ ioContext })
    {
        asyncResolveEndpoint(host, port, [this, host, port, onConnect](auto& error, auto endpoint) {
            if (error)
            {
                onConnect(error);
            }

            m_socket.async_connect(*endpoint, [this, endpoint, onConnect, host, port](const boost::system::error_code& error) {
                if (error)
                {
                    onConnect(error);
                    //throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
                }
                else
                {
                    setDefaultSocketOptions();
                    determineEndpoints();
                    onConnect(error);
                }
            });
        });
    }

    TcpChannel::TcpChannel(Channel::key_t key, boost::asio::ip::tcp::socket&& socket) :
        Channel(key),
        m_socket{ std::move(socket) },
        m_resolver( socket.get_executor()),
        m_localEndpoint{ "tcp://127.0.0.1" },
        m_remoteEndpoint{ m_localEndpoint },
        m_headerSize(0)
    {
        m_instanceBuffer.resize(8192);
        m_headerBuffer.resize(1024);

        if (m_socket.is_open())
        {
            determineEndpoints();
        }
    }

    const tools::Uri& TcpChannel::localEndpoint() const
    {
        return m_localEndpoint;
    }

    const tools::Uri& TcpChannel::remoteEndpoint() const
    {
        return m_remoteEndpoint;
    }

    void TcpChannel::asyncReceiveImpl()
    {
        asyncReadHeaderLength();
    }

    void TcpChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        std::string serializedInstance = to_cbor(instance, header.attributes);

        DotsTransportHeader transportHeader{
            DotsTransportHeader::dotsHeader_i{ header },
            DotsTransportHeader::payloadSize_i{ serializedInstance.size() }
        };

        // adjust header for backwards compatibility to legacy implementation
        {
            // always set destination group
            transportHeader.destinationGroup = transportHeader.dotsHeader->typeName;

            // conditionally set namespace
            if (instance._descriptor().internal() && !instance._is<DotsClient>() && !instance._is<DotsDescriptorRequest>())
            {
                transportHeader.nameSpace("SYS");
            }

            // set mandatory sent time if not valid
            if (!transportHeader.dotsHeader->sentTime.isValid())
            {
                transportHeader.dotsHeader->sentTime(types::timepoint_t::Now());
            }

            // set mandatory sender if not valid. note that a fixed server id for the sender can be used here because
            // in case of a client connection the id is handled on the server's side an will be overwritten anyway
            if (!transportHeader.dotsHeader->sender.isValid())
            {
               transportHeader.dotsHeader->sender = 1;
            }
        }

        auto serializedHeader = to_cbor(transportHeader);
        uint16_t headerSize = static_cast<uint16_t>(serializedHeader.size());

        std::array<boost::asio::const_buffer, 3> buffers{
            boost::asio::buffer(&headerSize, sizeof(headerSize)),
            boost::asio::buffer(serializedHeader.data(), serializedHeader.size()),
            boost::asio::buffer(serializedInstance.data(), serializedInstance.size())
        };

        m_socket.write_some(buffers);
    }

    void TcpChannel::setDefaultSocketOptions()
    {
        m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
        m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
        m_socket.set_option(boost::asio::socket_base::linger(true, 10));
    }

    void TcpChannel::determineEndpoints()
    {
        boost::asio::ip::tcp::endpoint localEndpoint = m_socket.local_endpoint();
        m_localEndpoint.setHost(localEndpoint.address().to_string());
        m_localEndpoint.setPort(std::to_string(localEndpoint.port()));

        boost::asio::ip::tcp::endpoint remoteEndpoint = m_socket.remote_endpoint();
        m_remoteEndpoint.setHost(remoteEndpoint.address().to_string());
        m_remoteEndpoint.setPort(std::to_string(remoteEndpoint.port()));
    }

    void TcpChannel::asyncReadHeaderLength()
    {
        boost::asio::async_read(m_socket, boost::asio::buffer(&m_headerSize, sizeof(m_headerSize)), [&, this_{ weak_from_this() }](auto ec, auto /*bytes*/)
        {
            try
            {
                if (this_.expired())
                {
                    return;
                }

                verifyErrorCode(ec);

                if (m_headerSize > m_headerBuffer.size())
                {
                    throw std::runtime_error{ "header buffer too small for header of size: " + std::to_string(m_headerSize) };
                }

                asyncReadHeader();
            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }

    void TcpChannel::asyncReadHeader()
    {
        boost::asio::async_read(m_socket, boost::asio::buffer(m_headerBuffer.data(), m_headerSize), [&, this_{ weak_from_this() }](auto ec, auto /*bytes*/)
        {
            try
            {
                if (this_.expired())
                {
                    return;
                }

                verifyErrorCode(ec);

                m_transportHeader = DotsTransportHeader{};
                from_cbor(&m_headerBuffer[0], m_headerSize, m_transportHeader);

                if (!m_transportHeader.payloadSize.isValid())
                {
                    throw std::runtime_error{ "received header without payloadSize" };
                }

                m_instanceBuffer.resize(m_transportHeader.payloadSize);
                asyncReadInstance();

            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }

    void TcpChannel::asyncReadInstance()
    {
        boost::asio::async_read(m_socket, boost::asio::buffer(m_instanceBuffer), [&, this_{ weak_from_this() }](auto ec, auto /*bytes*/)
        {
            try
            {
                if (this_.expired())
                {
                    return;
                }

                verifyErrorCode(ec);

                const type::StructDescriptor<>* descriptor = registry().findStructType(*m_transportHeader.dotsHeader->typeName).get();

                if (descriptor == nullptr)
                {
                    throw std::runtime_error{ "encountered unknown type: " + *m_transportHeader.dotsHeader->typeName };
                }

                type::AnyStruct instance{ *descriptor };
                from_cbor(m_instanceBuffer.data(), m_instanceBuffer.size(), instance.get());
                processReceive(Transmission{ std::move(m_transportHeader.dotsHeader), std::move(instance) });
            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }

    void TcpChannel::asyncResolveEndpoint(const std::string_view& host, const std::string_view& port, resolve_handler_t handler)
    {
        m_resolver.async_resolve(host, port, boost::asio::ip::resolver_query_base::numeric_service, [port, handler](const boost::system::error_code& error, auto iter) {
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

    void TcpChannel::verifyErrorCode(const boost::system::error_code& ec)
    {
        if (ec == boost::asio::error::misc_errors::eof || ec == boost::asio::error::basic_errors::bad_descriptor)
        {
            throw std::runtime_error{ "channel was closed unexpectedly: " + ec.message() };
        }
        else if (ec)
        {
            throw std::system_error{ ec };
        }
    }
}