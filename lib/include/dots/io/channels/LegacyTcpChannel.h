#pragma once
#include <dots/io/channels/AsyncStreamChannel.h>

namespace dots::io
{
    struct LegacyTcpChannel : AsyncStreamChannel<boost::asio::ip::tcp::socket, serialization::CborSerializer, TransmissionFormat::Legacy>
    {
        LegacyTcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, const Endpoint& endpoint);

        /**
         * Connect channel synchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         */
        LegacyTcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, std::string_view host, std::string_view port);

        /**
         * Connect channel asynchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         * @param onConnect
         */
        LegacyTcpChannel(Channel::key_t key, boost::asio::io_context& ioContext, std::string_view host, std::string_view port, std::function<void(const boost::system::error_code& error)> onConnect);

        /**
         * Construct channel with an already connected socket.
         * @param key
         * @param socket
         */
        LegacyTcpChannel(Channel::key_t key, boost::asio::ip::tcp::socket&& socket, payload_cache_t* payloadCache);
        LegacyTcpChannel(const LegacyTcpChannel& other) = delete;
        LegacyTcpChannel(LegacyTcpChannel&& other) = delete;
        ~LegacyTcpChannel() override = default;

        LegacyTcpChannel& operator = (const LegacyTcpChannel& rhs) = delete;
        LegacyTcpChannel& operator = (LegacyTcpChannel&& rhs) = delete;

    private:

        using resolve_handler_t = std::function<void(const boost::system::error_code& error, std::optional<boost::asio::ip::tcp::endpoint>)>;

        void asyncResolveEndpoint(std::string_view host, std::string_view port, resolve_handler_t handler);
        void verifyErrorCode(const boost::system::error_code& ec);
        void setDefaultSocketOptions();

        std::optional<receive_handler_t> m_cb;
        std::optional<error_handler_t> m_ecb;

        boost::asio::ip::tcp::resolver m_resolver;
    };
}
