#include "UdsChannel.h"
#include <csignal>
#include <dots/io/Io.h>
#include <dots/io/Registry.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots::io::posix
{
	UdsChannel::UdsChannel(asio::io_context& ioContext, const std::string_view& path) :
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

	UdsChannel::UdsChannel(asio::local::stream_protocol::socket&& socket) :
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

	void UdsChannel::transmitImpl(const DotsTransportHeader& header, const type::Struct& instance)
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

	void UdsChannel::asynReadHeaderLength()
	{
		asio::async_read(m_socket, asio::buffer(&m_headerSize, sizeof(m_headerSize)), [&](auto ec, auto /*bytes*/)
		{
			try
			{
				verifyErrorCode(ec);

				if (m_headerSize > m_headerBuffer.size())
				{
					throw std::runtime_error{ "header buffer too small for header of size: " + std::to_string(m_headerSize) };
				}

				asyncReadHeader();
			}
			catch (const std::exception& e)
			{
				processError(e);
			}
		});
	}

	void UdsChannel::asyncReadHeader()
	{
		asio::async_read(m_socket, asio::buffer(m_headerBuffer.data(), m_headerSize), [&](auto ec, auto /*bytes*/)
		{
			try
			{
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
			catch (const std::exception& e)
			{
				processError(e);
			}
		});
	}

	void UdsChannel::asyncReadInstance()
	{
		asio::async_read(m_socket, asio::buffer(m_instanceBuffer), [&](auto ec, auto /*bytes*/)
		{
			try
			{
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
			catch (const std::exception& e)
			{
				processError(e);
			}
		});
	}

	void UdsChannel::verifyErrorCode(const asio::error_code& ec)
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

    void UdsChannel::IgnorePipeSignals()
    {
		// ignores all pipe signals to prevent non-catchable application termination on broken pipes
		static auto IgnorePipesSignals = [](){ return std::signal(SIGPIPE, SIG_IGN); }();
		(void)IgnorePipesSignals;
    }
}