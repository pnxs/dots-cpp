#pragma once
#include <asio.hpp>

namespace dots
{
	struct IoContext : asio::io_context
	{
		IoContext(const IoContext& other) = delete;
		IoContext(IoContext&& other) = delete;
		~IoContext() = default;

		IoContext& operator = (const IoContext& rhs) = delete;
		IoContext& operator = (IoContext&& rhs) noexcept = delete; 

		static IoContext& Instance();

	private:

		IoContext() = default;
	};
}