#pragma once
#include <functional>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct Listener
	{
		using accept_handler_t = std::function<bool(channel_ptr_t)>;

		Listener() = default;
		Listener(const Listener& other) = delete;
		Listener(Listener&& other) = delete;
		virtual ~Listener() = default;

		Listener& operator = (const Listener& rhs) = delete;
		Listener& operator = (Listener&& rhs) = delete;

		void asyncAccept(accept_handler_t&& acceptHandler);

	protected:

		virtual void asyncAcceptImpl() = 0;
		void processAccept(channel_ptr_t channel);

	private:

		bool m_asyncAcceptActive = false;
		accept_handler_t m_acceptHandler;
	};
}