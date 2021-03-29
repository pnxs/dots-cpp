#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <dots/io/channels/UdsChannel.h>
#include <csignal>
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots::io::posix
{
    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const Endpoint& endpoint) :
        UdsChannel(key, ioContext, endpoint.path())
    {
        /* do nothing */
    }

    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& path) :
        Channel(key),
        m_socket{ ioContext },
        m_headerSize(0)
    {
        try
        {
            m_socket.connect(std::string{ path });
            initEndpoints(Endpoint{ "uds", m_socket.local_endpoint().path() }, Endpoint{ "uds", m_socket.local_endpoint().path() });
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "could not open UDS connection '" + std::string{ path } + "': " + e.what() };
        }

        m_instanceBuffer.resize(8192);
        m_headerBuffer.resize(1024);

        IgnorePipeSignals();
    }

    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::local::stream_protocol::socket&& socket) :
        Channel(key),
        m_socket{ std::move(socket) },
        m_headerSize(0)
    {
        m_instanceBuffer.resize(8192);
        m_headerBuffer.resize(1024);

        IgnorePipeSignals();

        if (m_socket.is_open())
        {
            initEndpoints(Endpoint{ "uds", m_socket.local_endpoint().path() }, Endpoint{ "uds", m_socket.local_endpoint().path() });
        }
    }

    void UdsChannel::asyncReceiveImpl()
    {
        asynReadHeaderLength();
    }

    void UdsChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        m_serializer.serialize(instance, header.attributes);
        std::vector<uint8_t> serializedInstance = std::move(m_serializer.output());

        DotsTransportHeader transportHeader{
            DotsTransportHeader::dotsHeader_i{ header },
            DotsTransportHeader::payloadSize_i{ serializedInstance.size() }
        };

        uint16_t serializedHeaderSize = static_cast<uint16_t>(m_serializer.serialize(transportHeader));
        std::vector<uint8_t> serializedHeader = std::move(m_serializer.output());

        std::array<boost::asio::const_buffer, 3> buffers{
            boost::asio::buffer(&serializedHeaderSize, sizeof(serializedHeaderSize)),
            boost::asio::buffer(serializedHeader.data(), serializedHeader.size()),
            boost::asio::buffer(serializedInstance.data(), serializedInstance.size())
        };

        m_socket.write_some(buffers);
        m_serializer.output().clear();
    }

    void UdsChannel::asynReadHeaderLength()
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

    void UdsChannel::asyncReadHeader()
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
                m_serializer.setInput(m_headerBuffer);
                m_transportHeader = m_serializer.deserialize<DotsTransportHeader>();

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

    void UdsChannel::asyncReadInstance()
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
                m_serializer.setInput(m_instanceBuffer);
                type::AnyStruct instance{ registry().getStructType(*m_transportHeader.dotsHeader->typeName) };
                m_serializer.deserialize(*instance);
                processReceive(Transmission{ std::move(m_transportHeader.dotsHeader), std::move(instance) });
            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }

    void UdsChannel::verifyErrorCode(const boost::system::error_code& ec)
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

    void UdsChannel::IgnorePipeSignals()
    {
        // ignores all pipe signals to prevent non-catchable application termination on broken pipes
        static auto IgnorePipesSignals = [](){ return std::signal(SIGPIPE, SIG_IGN); }();
        (void)IgnorePipesSignals;
    }
}
#endif