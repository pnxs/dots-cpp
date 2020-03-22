#pragma once
#include <functional>
#include <boost/asio.hpp>

namespace dots
{
	boost::asio::io_context& global_io_context();
	boost::asio::execution_context& global_execution_context();
	boost::asio::executor global_executor();

	template <typename Service>
	Service& global_service()
	{
		return boost::asio::use_service<Service>(global_execution_context());
	}
}