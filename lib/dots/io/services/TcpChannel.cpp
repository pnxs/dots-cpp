#include "TcpChannel.h"
#include <dots/io/Io.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots
{
	TcpChannel::TcpChannel(asio::io_context& ioContext, const std::string_view& host, const std::string_view& port) :
		TcpChannel(asio::ip::tcp::socket{ ioContext })
	{
		asio::ip::tcp::resolver resolver{ m_socket.get_executor().context() };
		auto endpoints = resolver.resolve(asio::ip::tcp::socket::protocol_type::v4(), host, port, asio::ip::resolver_query_base::numeric_service);

		for (const asio::ip::tcp::endpoint& endpoint: endpoints)
		{
			try
			{
				m_socket.connect(endpoint);

				m_socket.set_option(asio::ip::tcp::no_delay(true));
				m_socket.set_option(asio::ip::tcp::socket::keep_alive(true));
				m_socket.set_option(asio::socket_base::linger(true, 10));

				return;
			}
			catch (const std::exception&/* e*/)
			{
				/* do nothing */
			}
		}

		throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
	}

	TcpChannel::TcpChannel(asio::ip::tcp::socket&& socket) :		
		m_socket{ std::move(socket) },
		m_headerSize(0)
	{
		m_instanceBuffer.resize(8192);
		m_headerBuffer.resize(1024);
	}

	void TcpChannel::asyncReceiveImpl()
	{
		asynReadHeaderLength();
	}

	void TcpChannel::transmitImpl(const DotsTransportHeader& header, const type::Struct& instance)
	{
		std::string serializedInstance = to_cbor(instance, header.dotsHeader->attributes);

		DotsTransportHeader header_(header);
		header_.payloadSize = serializedInstance.size();

		auto serializedHeader = to_cbor(header_);
		uint16_t headerSize = serializedHeader.size();

		std::array<asio::const_buffer, 3> buffers{
			asio::buffer(&headerSize, sizeof(headerSize)),
			asio::buffer(serializedHeader.data(), serializedHeader.size()),
			asio::buffer(serializedInstance.data(), serializedInstance.size())
		};

		m_socket.write_some(buffers);
	}

	void TcpChannel::asynReadHeaderLength()
	{
		asio::async_read(m_socket, asio::buffer(&m_headerSize, sizeof(m_headerSize)), [&](auto ec, auto /*bytes*/)
		{
			if (ec)
			{
				handleError("error in header-length", ec);
				return;
			}

			if (m_headerSize > m_headerBuffer.size())
			{
				handleError("header-buffer to small for header of size " + std::to_string(m_headerSize), ec);
				return;
			}

			asyncReadHeader();
		});
	}

	void TcpChannel::asyncReadHeader()
	{
		asio::async_read(m_socket, asio::buffer(m_headerBuffer.data(), m_headerSize), [&](auto ec, auto bytes)
		{
			if (ec)
			{
				handleError("error in readHeader", ec);
				return;
			}

			try
			{
				// Decode header
				m_header = DotsTransportHeader{};

				from_cbor(&m_headerBuffer[0], m_headerSize, &m_header._Descriptor(), &m_header);

				string nameSpace = m_header.nameSpace.isValid() ? *m_header.nameSpace : "";
				bool remove = false;
				if (m_header.dotsHeader.isValid())
				{
					remove = m_header.dotsHeader->removeObj.isValid() ? m_header.dotsHeader->removeObj : false;
				}

				LOG_DEBUG_S("received header (size=" << bytes << "): ns=" << nameSpace << " dstGrp="
					<< *m_header.destinationGroup << " remove=" << remove
					<< " payloadSize=" << m_header.payloadSize);

				if (m_header.payloadSize.isValid())
				{
					m_instanceBuffer.resize(m_header.payloadSize);
					asyncReadInstance();
				}
				else
				{
					handleError("received header without payloadSize", ec);
				}
			}
			catch (const std::exception& e)
			{
				string msg = "exception in async-read handler: " + string(e.what());
				LOG_ERROR_S(msg);
				handleError(msg, std::make_error_code(std::errc::bad_message));
			}

		});
	}

	void TcpChannel::asyncReadInstance()
	{
		asio::async_read(m_socket, asio::buffer(m_instanceBuffer), [&](auto ec, auto bytes)
		{
			if (ec)
			{
				handleError("error in asyncReadInstance", ec);
				return;
			}
			LOG_DATA_S("received payload: " << m_instanceBuffer.size());

			if (const type::StructDescriptor* descriptor = type::Descriptor::registry().findStructDescriptor(m_header.dotsHeader->typeName); descriptor == nullptr)
			{
				// TODO: error handling
				throw std::runtime_error{ "unknown type: " + *m_header.dotsHeader->typeName };
			}
			else
			{
				type::AnyStruct instance{ *descriptor };
				from_cbor(m_instanceBuffer.data(), m_instanceBuffer.size(), descriptor, &instance.get());
				processReceive(m_header, Transmission{ std::move(instance) });
			}
		});
	}

	void TcpChannel::handleError(const string& text, const asio::error_code& ec)
	{
		if (ec == asio::error::misc_errors::eof || ec == asio::error::basic_errors::bad_descriptor)
		{
			processError(std::runtime_error{ "TCP channel was closed unexpectedly: " + text + ": " + ec.message() });
		}
		else
		{
			processError(std::runtime_error{ "TCP channel error: " + text + ": " + ec.message() });
		}
	}
}