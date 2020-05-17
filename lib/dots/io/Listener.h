#pragma once
#include <functional>
#include <memory>
#include <dots/io/Channel.h>

namespace dots::io
{
	struct Listener
	{
		using accept_handler_t = std::function<bool(Listener&, channel_ptr_t)>;
		using error_handler_t = std::function<void(Listener&, const std::exception_ptr&)>;

		Listener() = default;
		Listener(const Listener& other) = delete;
		Listener(Listener&& other) = delete;
		virtual ~Listener() = default;

		Listener& operator = (const Listener& rhs) = delete;
		Listener& operator = (Listener&& rhs) = delete;

		void asyncAccept(accept_handler_t&& acceptHandler, error_handler_t&& errorHandler);

	protected:

		virtual void asyncAcceptImpl() = 0;
		void processAccept(channel_ptr_t channel);
		void processError(const std::exception_ptr& e);
		void processError(const std::string& what);
		void verifyErrorCode(const std::error_code& errorCode);

	private:

		bool m_asyncAcceptActive = false;
		accept_handler_t m_acceptHandler;
		error_handler_t m_errorHandler;
	};

	using listener_ptr_t = std::unique_ptr<Listener>;
}