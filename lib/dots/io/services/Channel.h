#pragma once
#include <string>
#include <system_error>
#include <dots/io/services/Transmission.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io
{
	struct Registry;
}

namespace dots
{
	struct Channel : std::enable_shared_from_this<Channel>
	{
		using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&)>;
		using error_handler_t = std::function<void(const std::exception_ptr&)>;

		Channel() = default;
		Channel(const Channel& other) = delete;
		Channel(Channel&& other) = delete;
		virtual ~Channel() noexcept = default;

		Channel& operator = (const Channel& rhs) = delete;
		Channel& operator = (Channel&& rhs) = delete;

		void asyncReceive(io::Registry& registry, receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
		void transmit(const DotsTransportHeader& header, const Transmission& transmission);

		virtual bool isVirtual() { return false; }

	protected:

		const io::Registry& registry() const;
		io::Registry& registry();

		virtual void asyncReceiveImpl() = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const Transmission& transmission);

		void processReceive(const DotsTransportHeader& header, Transmission&& transmission) noexcept;
		void processError(const std::exception_ptr& e);
		void processError(const std::string& what);
		void verifyErrorCode(const std::error_code& errorCode);

	private:

		bool m_asyncReceiveActive = false;
		io::Registry* m_registry = nullptr;
		receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
	};

	using channel_ptr_t = std::shared_ptr<Channel>;
}