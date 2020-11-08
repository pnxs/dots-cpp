#pragma once
#include <string_view>
#include <optional>
#include <boost/asio.hpp>
#include <dots/io/Channel.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io
{
    struct TcpChannel : Channel
    {
        /**
         * Connect channel synchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         */
        TcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port);

        /**
         * Connect channel asynchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         * @param onConnect
         */
        TcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& host, const std::string_view& port, std::function<void(const boost::system::error_code& error)> onConnect);

        /**
         * Constuct channel with a already connected socket.
         * @param key
         * @param socket
         */
        TcpChannel(Channel::key_t key, boost::asio::ip::tcp::socket&& socket);
        TcpChannel(const TcpChannel& other) = delete;
        TcpChannel(TcpChannel&& other) = delete;
        virtual ~TcpChannel() = default;

        TcpChannel& operator = (const TcpChannel& rhs) = delete;
        TcpChannel& operator = (TcpChannel&& rhs) = delete;

        const Medium& medium() const override;

    protected:

        void asyncReceiveImpl() override;
        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override;

    private:

        static constexpr char TcpSocketCategory[] = "tcp";

	    void setDefaultSocketOptions();

        void asynReadHeaderLength();
        void asyncReadHeader();
        void asyncReadInstance();

        void asyncResolveEndpoint(const std::string_view& host, const std::string_view& port, std::function<void(const boost::system::error_code& error, std::optional<boost::asio::ip::tcp::endpoint>)> cb);

        void verifyErrorCode(const boost::system::error_code& error);

        receive_handler_t m_cb;
        error_handler_t m_ecb;

        boost::asio::ip::tcp::socket m_socket;
        boost::asio::ip::tcp::resolver m_resolver;
        uint16_t m_headerSize;
        DotsTransportHeader m_transportHeader;
        std::vector<uint8_t> m_headerBuffer;
        std::vector<uint8_t> m_instanceBuffer;
        std::optional<Medium> m_medium;
    };
}