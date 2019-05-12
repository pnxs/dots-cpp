#include "WebSocketChannel.h"
#include <dots/io/Io.h>
#include <dots/io/serialization/JsonSerializationRapidJson.h>

namespace dots
{
	WebSocketChannel::WebSocketChannel(asio::io_context& ioContext, const std::string_view& host, const std::string_view& port)
	{
		std::string uri = "ws://" + std::string{ host } + ":" + port.data();

		try
		{
			ws_client_t& client = m_client.emplace();
			client.set_access_channels(websocketpp::log::alevel::none);
        	client.set_error_channels(websocketpp::log::elevel::none);
			client.init_asio(&ioContext);

			websocketpp::lib::error_code ec;
			m_connection = client.get_connection(uri, ec);
			
			if (ec)
			{
				throw websocketpp::lib::system_error{ ec };
			}
			
			m_connection->add_subprotocol("dots");
			client.connect(m_connection);
		}
		catch (const std::exception& e)
		{
			throw std::runtime_error{ "could not open WebSocket connection: " + uri + ": " + e.what() };
		}		
	}

	WebSocketChannel::WebSocketChannel(ws_connection_ptr_t connection) :
        m_connection(connection)
	{
		/* do nothing */
	}

	void WebSocketChannel::asyncReceiveImpl()
	{
		m_connection->set_fail_handler([this](ws_connection_hdl_t hdl)
		{ 
			handleError("WebSocket channel encountered an error during async read");
		});

		m_connection->set_close_handler([this](ws_connection_hdl_t hdl)
		{ 
			handleError("WebSocket channel was closed unexpectedly");
		});

		m_connection->set_message_handler([this](ws_connection_hdl_t hdl, ws_message_ptr_t msg)
		{
			try
			{
				// TODO: optimize once JSON serializer has been reworked
				const std::string& payload = msg->get_payload();
				rapidjson::Document document;
				document.Parse(payload);

				if (document.HasParseError())
				{
					handleError("received invalid JSON: " + payload);
					return;
				}

				auto itHeader = document.FindMember("header");
				auto itInstance = document.FindMember("instance");

				if (itHeader == document.MemberEnd() || !itHeader->value.IsObject() || itInstance == document.MemberEnd() || !itInstance->value.IsObject())
				{
					handleError("received message has invalid format: " + payload);
					return;
				}

				DotsTransportHeader header;
				from_json(std::as_const(itHeader->value).GetObject(), &header._Descriptor(), &header);

				if (const type::StructDescriptor* descriptor = type::Descriptor::registry().findStructDescriptor(header.dotsHeader->typeName); descriptor == nullptr)
				{
					handleError("unknown type: " + *header.dotsHeader->typeName);
				}
				else
				{
					type::AnyStruct instance{ *descriptor };
					from_json(std::as_const(itInstance->value).GetObject(), descriptor, &instance.get());
					processReceive(header, Transmission{ std::move(instance) });
				}
			}
			catch (const std::exception& e)
			{
				handleError(e.what());
			}		
		});
	}

	void WebSocketChannel::transmitImpl(const DotsTransportHeader& header, const type::Struct& instance)
	{
		rapidjson::StringBuffer buffer;
    	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer{ buffer };

		// TODO: optimize once JSON serializer has been reworked
		writer.StartObject();
		{
			writer.String("header");
			dots::to_json(&header._Descriptor(), &header, writer);

			writer.String("instance");
			dots::to_json(&instance._descriptor(), &instance, writer);
		}
		writer.EndObject();

		if (websocketpp::lib::error_code ec = m_connection->send(buffer.GetString()))
		{
			throw websocketpp::lib::system_error{ ec };
		}
	}

	void WebSocketChannel::handleError(const std::string& what)
	{
		processError(std::runtime_error{ "WebSocket channel error: " + what });
	}
}