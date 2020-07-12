#include "WebSocketChannel.h"
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/JsonSerializationRapidJson.h>

namespace dots::io
{
    WebSocketChannel::WebSocketChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port) :
        Channel(key),
        m_stream{ ioContext }
    {
        try
        {
            boost::asio::ip::tcp::resolver resolver{ m_stream.get_executor() };
            auto endpoints = resolver.resolve(boost::asio::ip::tcp::socket::protocol_type::v4(), host, port, boost::asio::ip::resolver_query_base::numeric_service);
            auto& tcpStream = m_stream.next_layer();
            tcpStream.connect(endpoints.begin(), endpoints.end());
            m_medium.emplace(WebSocketCategory, tcpStream.socket().remote_endpoint().address().to_string());

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

    WebSocketChannel::WebSocketChannel(Channel::key_t key, ws_stream_t&& stream) :
        Channel(key),
        m_stream(std::move(stream))
    {
        m_medium.emplace(WebSocketCategory, m_stream.next_layer().socket().remote_endpoint().address().to_string());
    }

    const Medium& WebSocketChannel::medium() const
    {
        return *m_medium;
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

                // TODO: optimize once JSON serializer has been reworked
                const std::string& payload = boost::beast::buffers_to_string(m_buffer.cdata());
                rapidjson::Document document;
                document.Parse(payload);

                if (document.HasParseError())
                {
                    throw std::runtime_error{ "received invalid JSON: " + payload };
                }

                auto itHeader = document.FindMember("header");
                auto itInstance = document.FindMember("instance");

                if (itHeader == document.MemberEnd() || !itHeader->value.IsObject() || itInstance == document.MemberEnd() || !itInstance->value.IsObject())
                {
                    throw std::runtime_error{ "received message with invalid format: " + payload };
                }

                DotsHeader header;
                from_json(std::as_const(itHeader->value).GetObject(), header);

                const type::StructDescriptor<>* descriptor = registry().findStructType(*header.typeName).get();

                if (descriptor == nullptr)
                {
                    throw std::runtime_error{ "encountered unknown type: " + *header.typeName };
                }
                
                type::AnyStruct instance{ *descriptor };
                from_json(std::as_const(itInstance->value).GetObject(), instance.get());
                
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
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer{ buffer };

        // TODO: optimize once JSON serializer has been reworked
        writer.StartObject();
        {
            writer.String("header");
            dots::to_json(header, writer);

            writer.String("instance");
            dots::to_json(instance, writer);
        }
        writer.EndObject();
        
        m_stream.write(boost::asio::buffer(std::string{ buffer.GetString() }));
    }
}