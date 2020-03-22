#include "TcpChannel.h"
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots
{
	TcpChannel::TcpChannel(boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port) :
		TcpChannel(boost::asio::ip::tcp::socket{ ioContext })
	{
		boost::asio::ip::tcp::resolver resolver{ m_socket.get_executor() };
		auto endpoints = resolver.resolve(boost::asio::ip::tcp::socket::protocol_type::v4(), host, port, boost::asio::ip::resolver_query_base::numeric_service);

		for (const boost::asio::ip::tcp::endpoint& endpoint: endpoints)
		{
			try
			{
				m_socket.connect(endpoint);

				m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
				m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true));
				m_socket.set_option(boost::asio::socket_base::linger(true, 10));

				return;
			}
			catch (const std::exception&/* e*/)
			{
				/* do nothing */
			}
		}

		throw std::runtime_error{ "could not open TCP connection: " + std::string{ host } + ":" + std::string{ port } };
	}

	TcpChannel::TcpChannel(boost::asio::ip::tcp::socket&& socket) :		
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

		std::array<boost::asio::const_buffer, 3> buffers{
			boost::asio::buffer(&headerSize, sizeof(headerSize)),
			boost::asio::buffer(serializedHeader.data(), serializedHeader.size()),
			boost::asio::buffer(serializedInstance.data(), serializedInstance.size())
		};

		m_socket.write_some(buffers);
	}

	void TcpChannel::asynReadHeaderLength()
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

				m_header = DotsTransportHeader{};
				from_cbor(&m_headerBuffer[0], m_headerSize, m_header);

				if (!m_header.payloadSize.isValid())
				{
					throw std::runtime_error{ "received header without payloadSize" };
				}

				m_instanceBuffer.resize(m_header.payloadSize);
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

				const type::StructDescriptor<>* descriptor = registry().findStructType(*m_header.dotsHeader->typeName).get();

				if (descriptor == nullptr)
				{
					throw std::runtime_error{ "encountered unknown type: " + *m_header.dotsHeader->typeName };
				}

				type::AnyStruct instance{ *descriptor };
				from_cbor(m_instanceBuffer.data(), m_instanceBuffer.size(), instance.get());
				processReceive(m_header, Transmission{ std::move(instance) });
			}
			catch (...)
			{
				processError(std::current_exception());
			}
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