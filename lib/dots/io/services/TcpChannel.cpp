#include "TcpChannel.h"
#include <dots/io/Io.h>
#include <dots/io/serialization/CborNativeSerialization.h>

namespace dots
{
	TcpChannel::TcpChannel(asio::io_context& ioContext, const std::string& host, int port) :
		m_socket{ ioContext }
	{
		connect(host, port);
		m_buffer.resize(8192);
		m_headerBuffer.resize(1024);
	}

	TcpChannel::TcpChannel(asio::ip::tcp::socket&& socket)
		: m_socket(std::move(socket))
	{
		m_buffer.resize(8192);
		m_headerBuffer.resize(1024);
	}

	void TcpChannel::asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler)
	{
		m_cb = std::move(receiveHandler);
		m_ecb = std::move(errorHandler);
		readHeaderLength();
	}

	int TcpChannel::transmit(const DotsTransportHeader& header, const type::Struct& instance)
	{
		std::string payload = to_cbor(instance, header.dotsHeader->attributes);

		DotsTransportHeader _header(header);
		_header.payloadSize = payload.size();

		auto headerBuffer = to_cbor(_header);
		uint16_t headerSize = headerBuffer.size();

		std::array<asio::const_buffer, 3> buffers{
			asio::buffer(&headerSize, sizeof(headerSize)),
			asio::buffer(headerBuffer.data(), headerBuffer.size()),
			asio::buffer(payload.data(), payload.size())
		};

		m_socket.write_some(buffers);

		return 0;
	}

	bool TcpChannel::connect(const std::string& host, int port)
	{
		asio::ip::tcp::resolver resolver(m_socket.get_executor().context());
		auto iter = resolver.resolve({ host, "", asio::ip::resolver_query_base::numeric_service });
		decltype(iter) iterEnd;

		for (; iter != iterEnd; ++iter)
		{
			const auto& address = iter->endpoint().address();
			LOG_DEBUG_P("connect to %s (%s)", iter->host_name().c_str(), address.to_string().c_str());

			if (address.is_v4())
			{
				auto ep_to_string = [](const asio::ip::tcp::endpoint & ep)
				{
					return ep.address().to_string() + ":" + std::to_string(ep.port());
				};
				asio::ip::tcp::endpoint ep(address, port);
				asio::error_code ec;
				m_socket.connect(ep, ec);

				if (ec)
				{
					LOG_ERROR_P("unable to connect to socket '%s': %s",
						ep_to_string(ep).c_str(),
						ec.message().c_str());
					return false;
				}

				LOG_INFO_P("connected to socket '%s'", ep_to_string(ep).c_str());

				m_socket.set_option(asio::ip::tcp::no_delay(true), ec);
				m_socket.set_option(asio::ip::tcp::socket::keep_alive(true), ec);
				m_socket.set_option(asio::socket_base::linger(true, 10), ec);

				break;
			}
		}

		return true;
	}

	void TcpChannel::readHeaderLength()
	{
		asyncRead(asio::buffer(&m_headerSize, sizeof(m_headerSize)), [&](auto ec, auto /*bytes*/)
		{
			if (ec)
			{
				this->handleError("error in header-length", ec);
				return;
			}

			if (m_headerSize > m_headerBuffer.size())
			{
				this->handleError("header-buffer to small for header of size " + std::to_string(m_headerSize), ec);
				return;
			}

			this->readHeader();
		});
	}

	void TcpChannel::readHeader()
	{
		asyncRead(asio::buffer(&m_headerBuffer[0], m_headerSize), [&](auto ec, auto bytes)
		{
			if (ec)
			{
				this->handleError("error in readHeader", ec);
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
					m_payloadSize = m_header.payloadSize;
					this->readPayload();
				}
				else
				{
					this->handleError("received header without payloadSize", ec);
				}
			}
			catch (const std::exception& e)
			{
				string msg = "exception in async-read handler: " + string(e.what());
				LOG_ERROR_S(msg);
				this->handleError(msg, std::make_error_code(std::errc::bad_message));
			}

		});
	}

	void TcpChannel::readPayload()
	{
		m_buffer.resize(m_payloadSize);
		asyncRead(asio::buffer(&m_buffer[0], m_payloadSize), [&](auto ec, auto bytes)
		{
			if (ec)
			{
				this->handleError("error in readPayload", ec);
				return;
			}
			LOG_DATA_S("received payload: " << m_payloadSize);

			m_buffer.resize(bytes);

			if (m_cb)
			{
				if (const type::StructDescriptor* descriptor = type::Descriptor::registry().findStructDescriptor(m_header.dotsHeader->typeName); descriptor == nullptr)
				{
					// TODO: error handling
					throw std::runtime_error{ "unknown type: " + *m_header.dotsHeader->typeName };
				}
				else
				{
					type::AnyStruct instance{ *descriptor };
					from_cbor(m_buffer.data(), m_buffer.size(), descriptor, &instance.get());
					m_cb(m_header, std::move(instance), m_buffer);
				}
			}

			this->readHeaderLength();
		});
	}

	void TcpChannel::handleError(const string& text, const asio::error_code& ec)
	{
		int errorCode = 1;

		if (ec == asio::error::misc_errors::eof || ec == asio::error::basic_errors::bad_descriptor)
		{
			LOG_DEBUG_S(text << ": client closed connection");
			errorCode = 2;
		}
		else
		{
			LOG_ERROR_S("error " << text << " ec: " << ec);
		}

		if (m_ecb)
		{
			m_ecb(errorCode);
		}
	}
}