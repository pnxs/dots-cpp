#pragma once
#include <asio.hpp>
#include <dots/io/services/Listener.h>
#include <dots/io/services/Channel.h>

namespace dots
{
	struct ChannelService : asio::execution_context::service
	{
		using key_type = ChannelService;

		explicit ChannelService(asio::execution_context& executionContext) :
			asio::execution_context::service(executionContext)
		{
			/* do nothing */
		}
		ChannelService(const ChannelService& other) = delete;
		ChannelService(ChannelService&& other) noexcept = default;
		~ChannelService() = default;

		ChannelService& operator = (const ChannelService& rhs) = delete;
		ChannelService& operator = (ChannelService&& rhs) noexcept = default;

		template <typename TListener, typename... Args>
		std::unique_ptr<Listener> listen(Args&&... args)
		{
			return std::make_unique<TListener>(static_cast<asio::io_context&>(context()), std::forward<Args>(args)...);
		}

		template <typename TChannel, typename... Args>
		channel_ptr_t open(Args&&... args)
		{
			auto channel = std::make_shared<TChannel>(static_cast<asio::io_context&>(context()), std::forward<Args>(args)...);
			return channel;
		}

	private:

		void shutdown() noexcept override
		{
			/* do nothing */
		}
	};
}