#pragma once
#include <dots/io/channels/AsyncStreamChannel.h>

namespace dots::io
{
    struct TcpChannel : AsyncStreamChannel<asio::ip::tcp::socket>
    {
        TcpChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint);

        /**
         * Connect channel synchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         */
        TcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port);

        /**
         * Connect channel asynchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         * @param onConnect
         */
        TcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port, std::function<void(const boost::system::error_code& error)> onConnect);

        /**
         * Construct channel with an already connected socket.
         * @param key
         * @param socket
         */
        TcpChannel(key_t key, asio::ip::tcp::socket&& socket, payload_cache_t* payloadCache);
        TcpChannel(const TcpChannel& other) = delete;
        TcpChannel(TcpChannel&& other) = delete;
        ~TcpChannel() override = default;

        TcpChannel& operator = (const TcpChannel& rhs) = delete;
        TcpChannel& operator = (TcpChannel&& rhs) = delete;

    private:

        using resolve_handler_t = std::function<void(const boost::system::error_code& error, std::optional<asio::ip::tcp::endpoint>)>;

        void asyncResolveEndpoint(std::string_view host, std::string_view port, resolve_handler_t handler);
        void verifyErrorCode(const boost::system::error_code& ec);
        void setDefaultSocketOptions();

        std::optional<receive_handler_t> m_cb;
        std::optional<error_handler_t> m_ecb;

        asio::ip::tcp::resolver m_resolver;
    };
}
