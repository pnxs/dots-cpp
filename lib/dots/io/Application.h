#pragma once

#include "dots/cpp_config.h"
#include "Transceiver.h"

namespace dots
{

class IoService;

class Application
{
public:
    Application(const string& name, int& argc, char*argv[]);
    virtual ~Application();

    virtual int exec();
    virtual void exit(int exitCode = 0);

	boost::asio::io_service& ioService() const;

    static Application* instance();

private:
    void parseProgramOptions(int argc, char*argv[]);

    int m_exitCode = 0;

    inline static Application* m_instance = nullptr;

    std::string m_serverAddress;
    int m_serverPort;
};


}
