#pragma once
#include <string>
#include <vector>
#include <dots/io/services/Transmission.h>
#include <DotsTransportHeader.dots.h>

namespace dots
{
	struct Channel
	{
		using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&)>;
		using error_handler_t = std::function<void(int)>;

		Channel() = default;
		Channel(const Channel& other) = delete;
		Channel(Channel&& other) = delete;
		virtual ~Channel() = default;

		Channel& operator = (const Channel& rhs) = delete;
		Channel& operator = (Channel&& rhs) = delete;

		void asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
		void transmit(const DotsTransportHeader& header, const Transmission& transmission);

	protected:

		virtual void asyncReceiveImpl() = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const Transmission& transmission);

		void processReceive(const DotsTransportHeader& haeder, Transmission&& transmission);
		void processError(int ec);

	private:

		bool m_asyncReceiveActive = false;
		receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
	};

	using channel_ptr_t = std::shared_ptr<Channel>;
}