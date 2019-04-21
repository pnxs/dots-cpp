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
		using timer_id_t = Timer::timer_id_t;

		explicit TimerService(asio::execution_context& executionContext);
		TimerService(const TimerService& other) = delete;
		TimerService(TimerService&& other) noexcept = default;
		~TimerService() = default;

		TimerService& operator = (const TimerService& rhs) = delete;
		TimerService& operator = (TimerService&& rhs) noexcept = default;

		timer_id_t addTimer(const pnxs::chrono::Duration& timeout, const callback_t& cb, bool periodic);
		void removeTimer(timer_id_t id);

	private:

		void shutdown() noexcept override;

		inline static timer_id_t m_lastTimerId = 0;
		std::map<timer_id_t, Timer> m_timers;
	};
}