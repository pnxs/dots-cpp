#pragma once
#include <functional>
#include <map>
#include <asio.hpp>
#include <dots/io/services/FdHandler.h>

namespace dots
{
	struct FdEventService : asio::execution_context::service
	{
		using key_type = FdEventService;
		using callback_t = std::function<void()>;

		explicit FdEventService(asio::execution_context& executionContext);
		FdEventService(const FdEventService& other) = delete;
		FdEventService(FdEventService&& other) noexcept = default;
		~FdEventService() = default;

		FdEventService& operator = (const FdEventService& rhs) = delete;
		FdEventService& operator = (FdEventService&& rhs) noexcept = default;

		void addInEventHandler(int fileDescriptor, const callback_t& callback);
		void removeInEventHandler(int fileDescriptor);

	private:

		void shutdown() noexcept override;

		std::map<int, FdHandler> m_inEventHandlers;
	};
}