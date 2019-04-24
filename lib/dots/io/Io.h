#pragma once
#include <functional>
#include <asio.hpp>
#include <dots/common/Chrono.h>

namespace dots
{
	asio::io_context& global_io_context();
	asio::execution_context& global_execution_context();
	asio::executor global_executor();
}