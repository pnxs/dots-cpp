#include "AsioEventLoop.h"
#include <dots/eventloop/Timer.h>

namespace dots
{
	const asio::io_context& AsioEventLoop::ioContext() const
	{
		return m_ioContext;
	}

	asio::io_context& AsioEventLoop::ioContext()
	{
		return m_ioContext;
	}

	void AsioEventLoop::run()
	{
		m_ioContext.run();
	}

	void AsioEventLoop::runOne()
	{
		m_ioContext.run_one();
	}

	void AsioEventLoop::poll()
	{
		m_ioContext.poll();
	}

	void AsioEventLoop::pollOne()
	{
		m_ioContext.poll_one();
	}

	void AsioEventLoop::stop()
	{
		m_ioContext.stop();
	}

	auto AsioEventLoop::addTimer(const pnxs::chrono::Duration& timeout, const callback_t& cb, bool periodic) -> timer_id_t
	{
		timer_id_t id = ++m_lastTimerId;
		m_timers.try_emplace(id, m_ioContext, id, timeout, cb, periodic);

		return id;
	}

	void AsioEventLoop::removeTimer(unsigned id)
	{
		m_timers.erase(id);
	}

	void AsioEventLoop::addFdEventIn(int fd, const callback_t& cb)
	{
		m_fdHandlers[fd] = std::make_shared<AsioFdHandler>(m_ioContext, fd, cb);
	}

	void AsioEventLoop::removeFdEventIn(int fd)
	{
		if (auto it = m_fdHandlers.find(fd);  it != m_fdHandlers.end())
		{
			m_fdHandlers.erase(it);
		}
	}

	AsioEventLoop& AsioEventLoop::Instance()
	{
		static AsioEventLoop EventLoop;
		return EventLoop;
	}

	AsioEventLoop::AsioEventLoop()
	{
		pnxs::onAddTimer = [this](auto&&... args) { return addTimer(std::forward<decltype(args)>(args)...); };
		pnxs::onRemTimer = [this](timer_id_t timerId) { removeTimer(timerId); };
	}
}
