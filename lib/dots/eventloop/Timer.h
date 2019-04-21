#pragma once

#include <list>
#include <mutex>
#include <functional>
#include <dots/common/Chrono.h>
#include <dots/common/noncopyable.h>
#include <dots/io/IoContext.h>
#include <dots/io/services/TimerService.h>

namespace pnxs
{
typedef unsigned int TimerId;

/// install 'fun' as callback, to be executed 'timeout' later
/// if 'periodic' is true, timer will be automatically reinstalled after execution
/// returned TimerId can be used for timer removal with 'remTimer'
static inline
TimerId addTimer(const Duration &timeout, const std::function<void ()> &fun, bool periodic = false)
{
	return asio::use_service<dots::TimerService>(static_cast<asio::execution_context&>(dots::IoContext::Instance())).addTimer(timeout, fun, periodic);
}

/// remove timer with 'id' prior expiration
/// invocation with invalid (expired) 'id' will be silently ignored
static inline
void remTimer(TimerId id)
{
	asio::use_service<dots::TimerService>(static_cast<asio::execution_context&>(dots::IoContext::Instance())).removeTimer(id);
}

} // namespace pnxs
