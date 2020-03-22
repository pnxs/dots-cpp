#pragma once
#include <functional>
#include <utility>
#include <set>
#include <boost/asio.hpp>
#include <dots/io/services/Channel.h>
#include <DotsConnectionState.dots.h>

namespace dots
{
	struct VirtualChannel : Channel
	{
		VirtualChannel(boost::asio::io_context& ioContext, std::string serverName = "VirtualChannel", bool skipHandshake = false);
		VirtualChannel(boost::asio::ip::tcp::socket&& socket);
		VirtualChannel(const VirtualChannel& other) = delete;
		VirtualChannel(VirtualChannel&& other) = delete;
		virtual ~VirtualChannel() = default;

		VirtualChannel& operator = (const VirtualChannel& rhs) = delete;
		VirtualChannel& operator = (VirtualChannel&& rhs) = delete;

        void spoof(const DotsTransportHeader& header, const type::Struct& instance);
        void spoof(uint32_t sender, const type::Struct& instance, bool remove = false);

	protected:

		void asyncReceiveImpl() override;
		void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) override;

		virtual void onConnected();
		virtual void onSubscribe(const std::string& name);
		virtual void onUnsubscribe(const std::string& name);
		virtual void onTransmit(const DotsTransportHeader& header, const type::Struct& instance);

		const std::set<std::string>& subscribedTypes() const;

	private:
        static constexpr uint32_t ServerId = 1;
        static constexpr uint32_t ClientId = 2;

		std::reference_wrapper<boost::asio::io_context> m_ioContext;
		std::string m_serverName;
        DotsConnectionState m_connectionState;
		std::set<std::string> m_subscribedTypes;
        receive_handler_t m_receiveHandler;
	};
}