#pragma once
#include <functional>
#include <asio.hpp>

namespace dots
{
	asio::io_context& global_io_context();
	asio::execution_context& global_execution_context();
	asio::executor global_executor();
}