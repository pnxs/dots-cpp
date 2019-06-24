#pragma once
#include <functional>
#include <map>
#include <asio.hpp>
#include <dots/io/services/FdHandler.h>

namespace dots
{
	struct FdHandlerService : asio::execution_context::service
	{
		using key_type = FdHandlerService;
		using callback_t = std::function<void()>;

		explicit FdHandlerService(asio::execution_context& executionContext);
		FdHandlerService(const FdHandlerService& other) = delete;
		FdHandlerService(FdHandlerService&& other) noexcept(false) = default;
		~FdHandlerService() = default;

		FdHandlerService& operator = (const FdHandlerService& rhs) = delete;
		FdHandlerService& operator = (FdHandlerService&& rhs) noexcept(false) = default;

		void addInEventHandler(int fileDescriptor, const callback_t& callback);
		void removeInEventHandler(int fileDescriptor);

	private:

		void shutdown() noexcept override;

		std::map<int, FdHandler> m_inEventHandlers;
	};
}