#pragma once
#include <boost/asio.hpp>
#if defined(BOOST_ASIO_HAS_LOCAL_SOCKETS)
#include <string_view>
#include <dots/io/Channel.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io::posix
{
    struct UdsChannel : Channel
    {
        UdsChannel(Channel::key_t key, boost::asio::io_context& ioContext, const std::string_view& path);
        UdsChannel(Channel::key_t key, boost::asio::local::stream_protocol::socket&& socket);
        UdsChannel(const UdsChannel& other) = delete;
        UdsChannel(UdsChannel&& other) = delete;
        virtual ~UdsChannel() noexcept = default;

        UdsChannel& operator = (const UdsChannel& rhs) = delete;
        UdsChannel& operator = (UdsChannel&& rhs) = delete;

        const Medium& medium() const override;

    protected:

        void asyncReceiveImpl() override;
        void transmitImpl(const DotsHeader& header, const type::Struct& instance) override;

    private:

        static constexpr char UdsSocketCategory[] = "uds";

        void asynReadHeaderLength();
        void asyncReadHeader();
        void asyncReadInstance();

        void verifyErrorCode(const boost::system::error_code& error);

        static void IgnorePipeSignals();

        receive_handler_t m_cb;
        error_handler_t m_ecb;

        boost::asio::local::stream_protocol::endpoint m_endpoint;
        boost::asio::local::stream_protocol::socket m_socket;
        uint16_t m_headerSize;
        DotsTransportHeader m_transportHeader;
        std::vector<uint8_t> m_headerBuffer;
        std::vector<uint8_t> m_instanceBuffer;
        std::optional<Medium> m_medium;
    };
}
#else
#error "Local sockets are not available on this platform"
#endif