#include "AsioEventLoop.h"
#include <dots/eventloop/Timer.h>

namespace dots
{
	const boost::asio::io_service& AsioEventLoop::ioService() const
	{
		return m_ioService;
	}

	boost::asio::io_service& AsioEventLoop::ioService()
	{
		return m_ioService;
	}

	auto AsioEventLoop::addTimer(const pnxs::chrono::Duration& timeout, const callback_t& cb, bool periodic) -> timer_id_t
	{
		AsioTimer* timer = new AsioTimer(m_ioService, timeout, cb, periodic);
		return timer->id();
	}

	void AsioEventLoop::removeTimer(unsigned id)
	{
		AsioTimer::remTimer(id);
	}

	void AsioEventLoop::addFdEventIn(int fd, const callback_t& cb)
	{
		m_fdHandlers[fd] = std::make_shared<AsioFdHandler>(m_ioService, fd, cb);
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
