#pragma once

#include "dots/cpp_config.h"
#include "seconds.h"
#include "Chrono.h"
#include "Timer.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

namespace dots {

class AsioTimer
{
public:
    explicit AsioTimer(const function<void ()> &cb);
    virtual ~AsioTimer();
    AsioTimer( const AsioTimer & ) = delete;
    const AsioTimer & operator = ( const AsioTimer& ) = delete;

    void start(const pnxs::Duration & interval);
    static unsigned int singleShot(const pnxs::Duration & interval, const function<void ()> &cb);

private:
    void onTimeout(const boost::system::error_code& error);

    boost::asio::steady_timer m_timer;
    function<void ()> m_cb;
};

class AsioSingleShotTimer : public AsioTimer
{
    const function<void ()> m_cb;
    unsigned int m_id;

    static unsigned int m_lastTimerId;

    void callCb();

public:
    AsioSingleShotTimer(const pnxs::Duration & interval, const function<void ()> &cb);
    ~AsioSingleShotTimer() override;

    static std::map<unsigned int, AsioSingleShotTimer*> s_all;

    unsigned int id() { return m_id; }
    static void remTimer(unsigned int id);
};


}
