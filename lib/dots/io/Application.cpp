#include <dots/eventloop/Timer.h>
#include <dots/eventloop/AsioTimer.h>
#include "Application.h"
#include "dots/type/Registry.h"
#include "DotsAsioSocket.h"
#include <dots/eventloop/AsioFdHandler.h>
#include <boost/program_options.hpp>

#include "DotsClient.dots.h"
namespace po = boost::program_options;

namespace dots
{

pnxs::TimerId addTimerAsio(const pnxs::Duration& timeout, const function<void ()> &fun, bool /*periodic*/)
{
    AsioSingleShotTimer *timer = new AsioSingleShotTimer(timeout, fun);
    return timer->id();
}

void remTimerAsio(pnxs::TimerId id)
{
    AsioSingleShotTimer::remTimer(id);
}


Application* Application::m_instance = nullptr;

Application::Application(const string& name, int& argc, char*argv[])
    :m_ioService(dots::ioService())
{
    pnxs::onAddTimer = addTimerAsio;
    pnxs::onRemTimer = remTimerAsio;

    m_instance = this;

    parseProgramOptions(argc, argv);

    // Start Transceiver
    // Connect to dotsd
    auto dotsSocket = std::make_shared<DotsAsioSocket>();
    if(not transceiver().start(name, m_serverAddress, m_serverPort, dotsSocket))
    {
        throw std::runtime_error("unable to start transceiver");
        //quick_exit(-1);
    }

    LOG_DEBUG_S("run until state connected...");
    while(not transceiver().connected())
    {
        m_ioService.run_one();
    }
    LOG_DEBUG_S("run one done");

	DotsClient{ DotsClient::id_t_i{ transceiver().connection().clientId() }, DotsClient::running_t_i{ true } }._publish();
}

Application::~Application()
{
    // stop Transceiver
    transceiver().stop();
}

int Application::exec()
{
    // Check if connected

    m_exitCode = 0;

    // run mainloop
    m_ioService.run();

    return m_exitCode;
}

void Application::exit(int exitCode)
{
    m_exitCode = exitCode;
    // stop eventloop
    m_ioService.stop();
}

Application *Application::instance()
{
    return m_instance;
}

void Application::parseProgramOptions(int argc, char*argv[])
{
    po::options_description desc("Allowed options");
    desc.add_options()
            ("dots-address", po::value<string>()->default_value("127.0.0.1"), "address to bind to")
            ("dots-port", po::value<int>()->default_value(11234), "port to bind to")
            ;

    po::variables_map vm;
    po::store(po::basic_command_line_parser<char>(argc, argv).options(desc).allow_unregistered().run(), vm);
    po::notify(vm);

    // Check environment
    if (getenv("DOTS_SERVER_ADDRESS")) {
        m_serverAddress = getenv("DOTS_SERVER_ADDRESS");
    }

    if (getenv("DOTS_SERVER_PORT")) {
        m_serverPort = atoi("DOTS_SERVER_PORT");
    }

    m_serverAddress =vm["dots-address"].as<string>();
    m_serverPort = vm["dots-port"].as<int>();
}


}
