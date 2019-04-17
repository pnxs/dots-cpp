#pragma once
#include <functional>
#include <map>
#include <memory>
#include <boost/asio/io_service.hpp>
#include <dots/eventloop/AsioTimer.h>
#include <dots/eventloop/AsioFdHandler.h>

namespace dots
{
	struct AsioEventLoop
	{
		using callback_t = std::function<void()>;
		using timer_id_t = AsioTimer::timer_id_t;

		AsioEventLoop(const AsioEventLoop& other) = delete;
		AsioEventLoop(AsioEventLoop&& other) = delete;

		AsioEventLoop& operator = (const AsioEventLoop& rhs) = delete;
		AsioEventLoop& operator = (AsioEventLoop&& rhs) = delete;

		const boost::asio::io_service& ioService() const;
		boost::asio::io_service& ioService();

		void run();
		void runOne();

		void poll();
		void pollOne();

		void stop();

		timer_id_t addTimer(const pnxs::chrono::Duration& timeout, const callback_t& cb, bool periodic);
		void removeTimer(timer_id_t id);

		void addFdEventIn(int fd, const callback_t& cb);
		void removeFdEventIn(int fd);

		static AsioEventLoop& Instance();

	private:

		AsioEventLoop();
		~AsioEventLoop() = default;

		inline static timer_id_t m_lastTimerId = 0;		
		boost::asio::io_service m_ioService;
		std::map<timer_id_t, AsioTimer> m_timers;
		std::map<int, std::shared_ptr<AsioFdHandler>> m_fdHandlers;
	};
}