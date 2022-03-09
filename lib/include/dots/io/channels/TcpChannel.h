#pragma once
#include <dots/io/channels/AsyncStreamChannel.h>

namespace dots::io::details
{
    template <typename Serializer, TransmissionFormat TransmissionFormat>
    struct GenericTcpChannel : AsyncStreamChannel<asio::ip::tcp::socket, Serializer, TransmissionFormat>
    {
        using base_t = AsyncStreamChannel<asio::ip::tcp::socket, Serializer, TransmissionFormat>;
        using key_t = typename base_t::key_t;
        using payload_cache_t = typename base_t::payload_cache_t;

        GenericTcpChannel(key_t key, asio::io_context& ioContext, const Endpoint& endpoint);

        /**
         * Connect channel synchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         */
        GenericTcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port);

        /**
         * Connect channel asynchronously.
         * @param key
         * @param ioContext
         * @param host
         * @param port
         * @param onConnect
         */
        GenericTcpChannel(key_t key, asio::io_context& ioContext, std::string_view host, std::string_view port, std::function<void(const boost::system::error_code& error)> onConnect);

        /**
         * Construct channel with an already connected socket.
         * @param key
         * @param socket
         */
        GenericTcpChannel(key_t key, asio::ip::tcp::socket&& socket, payload_cache_t* payloadCache);
        GenericTcpChannel(const GenericTcpChannel& other) = delete;
        GenericTcpChannel(GenericTcpChannel&& other) = delete;
        ~GenericTcpChannel() override = default;

        GenericTcpChannel& operator = (const GenericTcpChannel& rhs) = delete;
        GenericTcpChannel& operator = (GenericTcpChannel&& rhs) = delete;

    private:

        using stream_t = typename base_t::stream_t;
        using receive_handler_t = typename base_t::receive_handler_t;
        using error_handler_t = typename base_t::error_handler_t;
        using resolve_handler_t = std::function<void(const boost::system::error_code& error, std::optional<asio::ip::tcp::endpoint>)>;

        using base_t::stream;
        using base_t::initEndpoints;

        void asyncResolveEndpoint(std::string_view host, std::string_view port, resolve_handler_t handler);
        void verifyErrorCode(const boost::system::error_code& ec);
        void setDefaultSocketOptions();

        std::optional<receive_handler_t> m_cb;
        std::optional<error_handler_t> m_ecb;

        asio::ip::tcp::resolver m_resolver;
    };

    extern template struct GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    extern template struct GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v2>;
}

namespace dots::io
{
    namespace v1
    {
        using TcpChannel = details::GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v1>;
    }

    inline namespace v2
    {
        using TcpChannel = details::GenericTcpChannel<serialization::CborSerializer, TransmissionFormat::v2>;
    }
}
