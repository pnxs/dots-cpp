#include <dots/io/channels/WebSocketChannel.h>
#include <dots/io/Io.h>
#include <dots/type/Registry.h>

namespace dots::io
{
    WebSocketChannel::WebSocketChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint) :
        WebSocketChannel(key, ioContext, endpoint.host(), endpoint.port())
    {
        /* do nothing */
    }

    WebSocketChannel::WebSocketChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port) :
        Channel(key),
        m_stream{ ioContext },
        m_serializer{ { serialization::TextOptions::Minified } }
    {
        try
        {
            asio::ip::tcp::resolver resolver{ m_stream.get_executor() };
            auto endpoints = resolver.resolve(asio::ip::tcp::socket::protocol_type::v4(), host, port, asio::ip::resolver_query_base::numeric_service);
            auto& tcpStream = m_stream.next_layer();
            tcpStream.connect(endpoints.begin(), endpoints.end());
            initEndpoints(Endpoint{ "ws", m_stream.next_layer().socket().local_endpoint() }, Endpoint{ "ws", m_stream.next_layer().socket().remote_endpoint() });

            m_stream.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));
            m_stream.set_option(boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type& req)
            {
                req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " DOTS WebSocket client");
                req.set(boost::beast::http::field::sec_websocket_protocol, Subprotocol);
            }));

            boost::beast::websocket::response_type res;
            m_stream.handshake(res, host.data(), "/");

            if (auto it = res.find(boost::beast::http::field::sec_websocket_protocol); it == res.end())
            {
                throw std::runtime_error{ "response is missing required subprotocol: " + std::string{ Subprotocol } };
            }
            else if (it->value() != Subprotocol)
            {
                throw std::runtime_error{ std::string{ "response has specified incompatible subprotocol: " } + std::string{ it->value().begin(), it->value().end() } + " != " + Subprotocol };
            }

            return;
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error{ "could not open WebSocket connection to '" + std::string{ host } + ":" + std::string{ port } + "': " + e.what() };
        }
    }

    WebSocketChannel::WebSocketChannel(key_t key, ws_stream_t&& stream) :
        Channel(key),
        m_stream(std::move(stream))
    {
        initEndpoints(Endpoint{ "ws", m_stream.next_layer().socket().local_endpoint() }, Endpoint{ "ws", m_stream.next_layer().socket().remote_endpoint() });
    }

    void WebSocketChannel::asyncReceiveImpl()
    {
        m_buffer.consume(m_buffer.size());
        m_stream.async_read(m_buffer, [&, this_{ weak_from_this() }](std::error_code ec, size_t/* bytes*/)
        {
            try
            {
                if (this_.expired())
                {
                    return;
                }

                verifyErrorCode(ec);

                m_serializer.setInput(static_cast<const char*>(m_buffer.cdata().data()), m_buffer.size());
                m_serializer.reader().readArrayBegin();
                DotsHeader header;
                m_serializer.deserialize(header);
                type::AnyStruct instance{ registry().getStructType(*header.typeName) };
                m_serializer.deserialize(*instance);
                m_serializer.reader().readArrayEnd();

                processReceive(Transmission{ std::move(header), std::move(instance) });
            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }

    void WebSocketChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        m_serializer.writer().writeArrayBegin();
        m_serializer.serialize(header);
        m_serializer.serialize(instance);
        m_serializer.writer().writeArrayEnd();

        m_stream.write(asio::buffer(m_serializer.output()));
        m_serializer.output().clear();
    }
}
