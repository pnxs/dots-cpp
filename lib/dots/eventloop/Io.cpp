#include "Io.h"
#include <boost/asio.hpp>
#include "AsioFdHandler.h"
#include "Timer.h"
#include "AsioTimer.h"

using namespace std::placeholders;

static pnxs::TimerId addTimerAsio(const pnxs::Duration& timeout, const std::function<void ()> &fun, bool /*periodic*/)
{
    dots::AsioSingleShotTimer *timer = new dots::AsioSingleShotTimer(timeout, fun);
    return timer->id();
}

static void remTimerAsio(pnxs::TimerId id)
{
    dots::AsioSingleShotTimer::remTimer(id);
}

namespace pnxs
{

AddFdEventFunction onAddEventIn;
RemoveFdEventFunction onRemoveEventIn;

AddFdEventFunction onAddEventOut;
RemoveFdEventFunction onRemoveEventOut;

AddFdEventFunction onAddEventExcept;
RemoveFdEventFunction onRemoveEventExect;

static std::map<int, std::shared_ptr<dots::AsioFdHandler>> g_fdhandler;

void addFdEventInAsio(boost::asio::io_service& io_service, int fd, const std::function<void ()> &fun)
{
    g_fdhandler[fd] = std::make_shared<dots::AsioFdHandler>(io_service, fd, fun);
}

void removeFdEventInAsio(boost::asio::io_service& /*io_service*/, int fd)
{
    auto iter = g_fdhandler.find(fd);
    if (iter != g_fdhandler.end()) {
        g_fdhandler.erase(iter);
    }
}

void ioInitAsio(boost::asio::io_service& io_service)
{
    pnxs::onAddEventIn    = std::bind(&addFdEventInAsio, std::ref(io_service), _1, _2);
    pnxs::onRemoveEventIn = std::bind(&removeFdEventInAsio, std::ref(io_service), _1);

    pnxs::onAddTimer = addTimerAsio;
    pnxs::onRemTimer = remTimerAsio;
}


}