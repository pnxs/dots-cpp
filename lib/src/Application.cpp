// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#undef DOTS_NO_GLOBAL_TRANSCEIVER
#include <dots/Application.h>
#include <dots/io/Io.h>
#include <boost/program_options.hpp>
#include <dots/tools/logging.h>
#include <DotsClient.dots.h>

namespace dots
{
    struct Application::signal_set_storage
    {
        signal_set_storage(asio::io_context& ioContext) : signalSet{ ioContext, SIGINT, SIGTERM } {}
        asio::signal_set signalSet;
    };

    Application::Application(const std::string& name, int argc, char* argv[], std::optional<GuestTransceiver> guestTransceiver/* = std::nullopt*/, bool handleExitSignals/* = true*/) :
        m_exitCode(EXIT_SUCCESS),
        m_transceiver(nullptr),
        m_guestTransceiverStorage{ std::move(guestTransceiver) }
    {
        parseGuestTransceiverArgs(argc, argv);
        GuestTransceiver* transceiver;

        if (m_guestTransceiverStorage == std::nullopt || &*m_guestTransceiverStorage == &*global_transceiver())
        {
            using transition_handler_t = GuestTransceiver::transition_handler_t;
            transceiver = &global_transceiver().emplace(
                m_openEndpoint->userName().empty() ? name : std::string{ m_openEndpoint->userName() },
                io::global_io_context(),
                type::Registry::StaticTypePolicy::All,
                transition_handler_t{ &Application::handleGuestTransceiverTransition, this }
            );
            transceiver->open(io::global_publish_types(), io::global_subscribe_types(), *m_openEndpoint);
        }
        else
        {
            transceiver = &*m_guestTransceiverStorage;
            transceiver->open(*m_openEndpoint);
        }

        m_transceiver = transceiver;

        if (handleExitSignals)
        {
            m_signals = std::make_unique<signal_set_storage>(ioContext());
            m_signals->signalSet.async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }

        for (;;)
        {
            if (m_guestConnectionError != nullptr)
            {
                std::rethrow_exception(m_guestConnectionError);
            }

            if (transceiver->connected())
            {
                break;
            }
            else
            {
                ioContext().run_one();
            }
        }

        transceiver->publish(DotsClient{ .id = transceiver->connection().selfId(), .running = true });
    }

    Application::Application(const std::string& name, std::optional<GuestTransceiver> guestTransceiver/* = std::nullopt*/, bool handleExitSignals/* = true*/) :
        Application(name, 0, nullptr, std::move(guestTransceiver), handleExitSignals)
    {
        /* do nothing */
    }

    Application::Application(int argc, char* argv[], HostTransceiver hostTransceiver, bool handleExitSignals) :
        m_exitCode(EXIT_SUCCESS),
        m_transceiver(nullptr),
        m_hostTransceiverStorage{ std::move(hostTransceiver) }
    {
        parseHostTransceiverArgs(argc, argv);
        m_transceiver = &*m_hostTransceiverStorage;
        m_hostTransceiverStorage->listen(m_listenEndpoints);

        if (handleExitSignals)
        {
            m_signals = std::make_unique<signal_set_storage>(ioContext());
            m_signals->signalSet.async_wait([this](boost::system::error_code/* error*/, int/* signalNumber*/){ exit(); });
        }
    }

    Application::Application(HostTransceiver hostTransceiver, bool handleExitSignals/* = true*/) :
        Application(0, nullptr, std::move(hostTransceiver), handleExitSignals)
    {
        /* do nothing */
    }

    Application::~Application()
    {
        ioContext().stop();

        if (m_hostTransceiverStorage != std::nullopt)
        {
            m_hostTransceiverStorage = std::nullopt;
        }
        else if (m_guestTransceiverStorage != std::nullopt)
        {
            m_guestTransceiverStorage = std::nullopt;
        }
        else
        {
            global_transceiver().reset();
        }

        ioContext().restart();
        ioContext().poll();
    }

    const Transceiver& Application::transceiver() const
    {
        return *m_transceiver;
    }

    Transceiver& Application::transceiver()
    {
        return *m_transceiver;
    }

    int Application::exec()
    {
        m_exitCode = EXIT_SUCCESS;
        ioContext().run();

        return m_exitCode;
    }

    void Application::exit(int exitCode)
    {
        m_exitCode = exitCode;
        ioContext().stop();
    }

    const asio::io_context& Application::ioContext() const
    {
        return m_transceiver->ioContext();
    }

    asio::io_context& Application::ioContext()
    {
        return m_transceiver->ioContext();
    }

    void Application::handleGuestTransceiverTransition(const Connection& connection, std::exception_ptr ePtr)
    {
        m_guestConnectionError = ePtr;

        if (connection.closed())
        {
            exit();
        }
    }

    void Application::parseGuestTransceiverArgs(int argc, char* argv[])
    {
        namespace po = boost::program_options;

        po::options_description options("Allowed options");
        options.add_options()
            ("dots-auth-secret", po::value<std::string>(), "secret used during authentication (this can also be given as part of the --dots-endpoint argument)")
            ("dots-endpoint", po::value<std::string>(), "remote endpoint URI to open for host connection (e.g. tcp://127.0.0.1, ws://127.0.0.1:11233, uds:/run/dots.socket")
            ("dots-log-level", po::value<int>(), "log level to use (data = 1, debug = 2, info = 3, notice = 4, warn = 5, error = 6, crit = 7, emerg = 8)")
        ;

        po::variables_map args;

        if (argc > 0 && argv != nullptr)
        {
            po::store(po::basic_command_line_parser<char>(argc, argv).options(options).allow_unregistered().run(), args);
            po::notify(args);
        }

        if (auto it = args.find("dots-endpoint"); it != args.end())
        {
            m_openEndpoint.emplace(it->second.as<std::string>());
        }
        else if (const char* openEndpointUri = ::getenv("DOTS_ENDPOINT"); openEndpointUri != nullptr)
        {
            m_openEndpoint.emplace(openEndpointUri);
        }
        else
        {
             m_openEndpoint.emplace("tcp://127.0.0.1");
        }

        if ((m_openEndpoint->scheme() == "tcp" || m_openEndpoint->scheme() == "tcp-v2") && m_openEndpoint->port().empty())
        {
            m_openEndpoint->setPort("11235");
        }
        else if (m_openEndpoint->scheme() == "tcp-v1" && m_openEndpoint->port().empty())
        {
            m_openEndpoint->setPort("11234");
        }

        if (auto it = args.find("dots-auth-secret"); it != args.end())
        {
            m_openEndpoint->setUserPassword(it->second.as<std::string>());
        }
        else if (const char* dotsAuthSecret = ::getenv("DOTS_AUTH_SECRET"); dotsAuthSecret != nullptr)
        {
            m_openEndpoint->setUserPassword(dotsAuthSecret);
        }

        if (auto it = args.find("dots-log-level"); it != args.end())
        {
            tools::loggingFrontend().setLogLevel(it->second.as<int>());
        }
    }

    void Application::parseHostTransceiverArgs(int argc, char* argv[])
    {
        namespace po = boost::program_options;

        po::options_description options{ "Allowed options" };
        options.add_options()
            ("dots-endpoint", po::value<std::vector<std::string>>(), "local endpoint URI to listen on for incoming guest connections (e.g. tcp://127.0.0.1, ws://127.0.0.1:11233, uds:/run/dots.socket")
            ("dots-log-level", po::value<int>(), "log level to use (data = 1, debug = 2, info = 3, notice = 4, warn = 5, error = 6, crit = 7, emerg = 8)")
        ;

        po::variables_map args;

        if (argc > 0 && argv != nullptr)
        {
            po::store(po::basic_command_line_parser<char>(argc, argv).options(options).allow_unregistered().run(), args);
            po::notify(args);
        }

        if (auto it = args.find("dots-endpoint"); it != args.end())
        {
            for (const std::string& listenEndpointUri : it->second.as<std::vector<std::string>>())
            {
                m_listenEndpoints.emplace_back(listenEndpointUri);
            }
        }
        else if (const char* listenEndpointUris = ::getenv("DOTS_ENDPOINT"); listenEndpointUris != nullptr)
        {
            m_listenEndpoints = io::Endpoint::FromStrings(listenEndpointUris);
        }
        else
        {
            m_listenEndpoints.emplace_back("tcp://127.0.0.1");
        }

        for (io::Endpoint& listenEndpoint : m_listenEndpoints)
        {
            if ((listenEndpoint.scheme() == "tcp" || listenEndpoint.scheme() == "tcp-v2") && listenEndpoint.port().empty())
            {
                listenEndpoint.setPort("11235");
            }
            else if (listenEndpoint.scheme() == "tcp-v1" && listenEndpoint.port().empty())
            {
                listenEndpoint.setPort("11234");
            }
        }

        if (auto it = args.find("dots-log-level"); it != args.end())
        {
            tools::loggingFrontend().setLogLevel(it->second.as<int>());
        }
    }
}
