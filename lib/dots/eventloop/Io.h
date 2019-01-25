#pragma once

#include <functional>

namespace boost{
namespace asio {
class io_service;
}
}

namespace pnxs
{

void ioInitAsio(boost::asio::io_service& io_service);

typedef std::function<void (int fd, const std::function<void ()> &fun)> AddFdEventFunction;
typedef std::function<void (int fd)> RemoveFdEventFunction;

extern AddFdEventFunction onAddEventIn;
extern RemoveFdEventFunction onRemoveEventIn;

extern AddFdEventFunction onAddEventOut;
extern RemoveFdEventFunction onRemoveEventOut;

extern AddFdEventFunction onAddEventExcept;
extern RemoveFdEventFunction onRemoveEventExect;

static inline
void addFdEventIn(int fd, const std::function<void ()> &fun)
{
    return onAddEventIn(fd, fun);
}

static inline
void removeFdEventIn(int fd)
{
    return onRemoveEventIn(fd);
}


static inline
void addFdEventOut(int fd, const std::function<void ()> &fun)
{
    return onAddEventOut(fd, fun);
}

static inline
void removeFdEventOut(int fd)
{
    return onRemoveEventIn(fd);
}


static inline
void addFdEventExecpt(int fd, const std::function<void ()> &fun)
{
    return onAddEventExcept(fd, fun);
}

static inline
void removeFdEventExcept(int fd)
{
    return onRemoveEventExect(fd);
}

}