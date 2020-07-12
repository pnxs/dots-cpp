#include "UdsChannel.h"
#include <csignal>
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots::io::posix
{
    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& path) :
        Channel(key),
        m_endpoint{ path.data() },
        m_socket{ ioContext },
        m_headerSize(0)
    {
        try
        {
            m_socket.connect(m_endpoint);
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "could not open UDS connection '" + m_endpoint.path() + "': " + e.what() };
        }

        m_instanceBuffer.resize(8192);
        m_headerBuffer.resize(1024);

        IgnorePipeSignals();
    }

    UdsChannel::UdsChannel(Channel::key_t key, boost::asio::local::stream_protocol::socket&& socket) :
        Channel(key),
        m_endpoint{ socket.remote_endpoint() },
        m_socket{ std::move(socket) },
        m_headerSize(0)
    {
        m_instanceBuffer.resize(8192);
        m_headerBuffer.resize(1024);

        IgnorePipeSignals();
    }

    void UdsChannel::asyncReceiveImpl()
    {
        asynReadHeaderLength();
    }

    void UdsChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        std::string serializedInstance = to_cbor(instance, header.attributes);

        DotsTransportHeader transportHeader{
            DotsTransportHeader::dotsHeader_i{ header },
            DotsTransportHeader::payloadSize_i{ serializedInstance.size() }
        };

        auto serializedHeader = to_cbor(transportHeader);
        uint16_t headerSize = serializedHeader.size();

        std::array<boost::asio::const_buffer, 3> buffers{
            boost::asio::buffer(&headerSize, sizeof(headerSize)),
            boost::asio::buffer(serializedHeader.data(), serializedHeader.size()),
            boost::asio::buffer(serializedInstance.data(), serializedInstance.size())
        };

        m_socket.write_some(buffers);
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