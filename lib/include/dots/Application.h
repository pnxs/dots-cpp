#pragma once
#include <chrono>
#include <optional>
#include <dots/dots.h>
#include <dots/GuestTransceiver.h>
#include <dots/io/Endpoint.h>

namespace dots
{
    /*!
     * @brief Top-level helper class for DOTS applications.
     *
     * The Application class is intended to be used in the main() function
     * of an application to ease the initialization of a transceiver and
     * handling of the event loop.
     *
     * It defines a set of default command line options that are used to
     * initialize a transceiver and open a connection based on the
     * arguments given.
     *
     * This class is mostly intended to be used in conjunction with the
     * global DOTS API (see dots.h):
     *
     * @code{.cpp}
     * #include <dots/Application.h>
     *
     * int main(int argc, char* argv[])
     * {
     *     dots::Application app{ "<app-name>", argc, argv };
     *     dots::subscribe<Foo>([](const dots::Event<Foo>& event)
     *     {
     *         // ...
     *     });
     *     // ...
     *     return app.exec();
     * }
     * @endcode
     */
    struct Application
    {
        /*!
         * @brief Construct a new Application object.
         *
         * This will parse the given command line arguments and attempt to
         * establish a connection via the given transceiver using the endpoint
         * given by the '--dots-open' option. If no endpoint is specified,
         * "tcp://127.0.0.1:11234" will be used as a default.
         *
         * If no transceiver is given (i.e. the global transceiver is used) and
         * any of the statically typed versions of dots::subscribe<T>() or
         * dots::container<T>() of the global DOTS API were instantiated (see
         * dots.h), establishing the connection will include preloading of each
         * type used as arguments to those templates.
         *
         * This means that when the constructor returns, the cache (i.e. the
         * containers) of those types will have been updated to the latest
         * state known to the host.
         *
         * Note that creating the connection will be performed synchronously.
         * In other words, the constructor will block and execute the event
         * loop until the connection, including preloading, has been
         * established:
         *
         * @code{.cpp}
         * dots::Application app{ "<app-name>", argc, argv };
         * // cache for Foo and Bar will have been preloaded here
         * dots::subscribe<Foo>([](const dots::Event<Foo>& event)
         * {
         *     auto& container = dots::container<Bar>();
         *     // ...
         * });
         * // ...
         * @endcode
         *
         * @param name The name that will be used by the Transceiver to
         * identify itself.
         *
         * @param argc The number of command line arguments as given in the
         * main() function of the application.
         *
         * @param argv The command line arguments as given in the main()
         * function of the application.
         *
         * @param transceiver The transceiver the application will operate on.
         * If none is given, the global guest transceiver will be used.
         *
         * @param handleExitSignals Indicates whether the application should
         * exit on SIGINT and SIGTERM signals.
         *
         * @exception std::exception Thrown if no connection could be
         * established based on the given arguments.
         */
        Application(const std::string& name, int argc, char* argv[], std::optional<GuestTransceiver> transceiver = std::nullopt, bool handleExitSignals = true);

        Application(const Application& other) = delete;
        Application(Application&& other) = delete;

        /*!
         * @brief Destroy the Application object.
         *
         * If the application is still running, this will stop the event loop
         * as if Application::exit() were called.
         */
        virtual ~Application();

        Application& operator = (const Application& rhs) = delete;
        Application& operator = (Application&& rhs) = delete;

        /*!
         * @brief Get the transceiver the application operates on.
         *
         * Note that this is the same transceiver that was specified in
         * Application() or the global guest transceiver if none was provided.
         *
         * @return const Transceiver& A reference to the used transceiver.
         */
        const Transceiver& transceiver() const;

        /*!
         * @brief Get the transceiver the application operates on.
         *
         * Note that this is the same transceiver that was specified in
         * Application() or the global guest transceiver if none was provided.
         *
         * @return Transceiver& A reference to the used transceiver.
         */
        Transceiver& transceiver();

        /*!
         * @brief Execute the application.
         *
         * This will block and run the event loop (i.e. run the IO context of
         * the given transceiver) until the Application has exited (see
         * Application::exit()) or all work has finished.
         *
         * @return int The exit code of the application as passed in
         * Application::exit().
         */
        virtual int exec();

        /*!
         * @brief Exit the application.
         *
         * This will send an exit signal to the application and set a given
         * exit code.
         *
         * It is intended to be used from within an asynchronous handler to
         * stop the event loop.
         *
         * This will have no effect when the application is not running.
         *
         * Note that the application cannot be executed again after this
         * function has been called.
         *
         * @param exitCode The exit code to return in Application::exec() and
         * Application::execOne().
         */
        virtual void exit(int exitCode = EXIT_SUCCESS);

    private:
        
        const asio::io_context& ioContext() const;
        asio::io_context& ioContext();

        void parseProgramOptions(int argc, char* argv[]);

        std::optional<io::Endpoint> m_openEndpoint;
        std::optional<asio::signal_set> m_signals;
        int m_exitCode;
        GuestTransceiver* m_transceiver;
        std::optional<GuestTransceiver> m_transceiverStorage;
    };
}
