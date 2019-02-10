#include "DotsAsioSocket.h"

#include "TcpResolver.h"
#include "TcpEndpoint.h"
#include "dots/io/serialization/CborNativeSerialization.h"

namespace dots
{

void DotsAsioSocket::start()
{
    m_buffer.resize(8192);
    m_headerBuffer.resize(1024); //TODO: may be smaller...
    readHeaderLength();
}

void DotsAsioSocket::setReceiveCallback(DotsSocket::receive_callback cb)
{
    m_cb = cb;
}

int DotsAsioSocket::send(const DotsTransportHeader &header, const vector<uint8_t> &data)
{
    DotsTransportHeader _header(header);
    _header.payloadSize(data.size());

    auto headerBuffer = to_cbor(_header);

    uint16_t headerSize = headerBuffer.size();

    std::array<boost::asio::const_buffer, 3> buffers{
        boost::asio::buffer(&headerSize, sizeof(headerSize)),
        boost::asio::buffer(headerBuffer.data(), headerBuffer.size()),
        boost::asio::buffer(&data[0], data.size())
    };
    m_socket.write_some(buffers);

    return 0;
}

bool DotsAsioSocket::connect(const string &host, int port)
{
    TcpResolver resolver(ioService());
    auto iter = resolver.resolve({host, "", boost::asio::ip::resolver_query_base::numeric_host
        | boost::asio::ip::resolver_query_base::numeric_service});
    decltype(iter) iterEnd;

    for (; iter != iterEnd; ++iter)
    {
        const auto &address = iter->endpoint().address();
        LOG_DEBUG_P("connect to %s (%s)", iter->host_name().c_str(), address.to_string().c_str());

        if (address.is_v4())
        {
            TcpEndpoint ep(address, port);
            m_socket.connect(ep, m_socket.ec);

            if (m_socket.ec)
            {
                LOG_ERROR_P("unable to connect to socket '%s': %s",
                            ep.toString().c_str(),
                            m_socket.ec.message().c_str());
                return false;
            }

            LOG_INFO_P("connected to socket '%s'", ep.toString().c_str());

            boost::system::error_code ec;
            m_socket.set_option(boost::asio::ip::tcp::no_delay(true), ec);
            m_socket.set_option(boost::asio::ip::tcp::socket::keep_alive(true), ec);
            m_socket.set_option(boost::asio::socket_base::linger(true, 10), ec);

            break;
        }

    }

    start();

    //onConnected();

    return true;
}

void DotsAsioSocket::disconnect()
{
    m_socket.close();
}

void DotsAsioSocket::readHeaderLength()
{
    //LOG_DEBUG_S("start readHeaderLength");
    m_socket.asyncRead(boost::asio::buffer(&m_headerSize, sizeof(m_headerSize)), [&](auto ec, auto bytes)
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

void DotsAsioSocket::readHeader()
{
    m_socket.asyncRead(boost::asio::buffer(&m_headerBuffer[0], m_headerSize), [&](auto ec, auto bytes)
    {
        if (ec)
        {
            this->handleError("error in readHeader", ec);
            return;
        }

        try
        {
            // Decode header
			m_header = {};

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
        catch (const std::exception &e)
        {
            string msg = "exception in async-read handler: " + string(e.what());

            // Need include "buffer.h" to work:
            //LOG_ERROR_S("Headersize: " << m_headerSize);
            //LOG_ERROR_S("Headerdata:" << cbor::hexlify(&m_headerBuffer[0], m_headerSize));
            //LOG_ERROR_S("Ec: " << ec);
            //LOG_ERROR_S("Bytes: " << bytes);

            LOG_ERROR_S(msg);
            boost::system::error_code bmec(boost::system::errc::bad_message, boost::system::generic_category());
            this->handleError(msg, bmec);
        }

    });
}

void DotsAsioSocket::readPayload()
{
    m_buffer.resize(m_payloadSize);
    m_socket.asyncRead(boost::asio::buffer(&m_buffer[0], m_payloadSize), [&](auto ec, auto bytes)
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
            m_cb(Message(m_header, m_buffer));
        }

        this->readHeaderLength();
    });
}

void DotsAsioSocket::setErrorCallback(DotsSocket::error_callback cb)
{
    m_ecb = cb;
}

void DotsAsioSocket::handleError(const string &text, const boost::system::error_code &ec)
{
    int errorCode = 1;

    if (ec == boost::asio::error::misc_errors::eof || ec == boost::system::errc::bad_file_descriptor)
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

DotsAsioSocket::DotsAsioSocket(TcpSocket socket)
    : m_socket(std::move(socket))
{
    start();
}

}