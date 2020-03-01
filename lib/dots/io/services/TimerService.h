#pragma once
#include <functional>
#include <map>
#include <asio.hpp>
#include <dots/io/services/Timer.h>

namespace dots
{
	struct TimerService : asio::execution_context::service
	{
		using key_type = TimerService;
		using callback_t = std::function<void()>;

		explicit TimerService(asio::execution_context& executionContext);
		TimerService(const TimerService& other) = delete;
		TimerService(TimerService&& other) noexcept(false) = delete;
		~TimerService() = default;

		TimerService& operator = (const TimerService& rhs) = delete;
		TimerService& operator = (TimerService&& rhs) noexcept(false) = delete;

		Timer::id_t addTimer(const type::Duration& timeout, const callback_t& cb, bool periodic);
		void removeTimer(Timer::id_t id);

	private:

		void shutdown() noexcept override;

		inline static Timer::id_t m_lastTimerId = 0;
		std::map<Timer::id_t, Timer> m_timers;
	};
}