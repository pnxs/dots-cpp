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
     * initialize the global transceiver and open a connection based on the
     * arguments given.
     *
     * This class is intended to be used in conjunction with the global
     * DOTS API (see dots.h):
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
     *
     * @warning The Application is currently not designed to be created
     * multiple times or used in scenarios other than in the example given
     * above.
     */
    struct Application
    {
        /*!
         * @brief Construct a new Application object.
         *
         * This will parse the given command line arguments and attempt to
         * establish a connection via the global transceiver using the endpoint
         * given by the '--dots-open' option. If no endpoint is specified,
         * "tcp://127.0.0.1:11234" will be used as a default.
         *
         * If any of the statically typed versions of dots::subscribe<T>() or
         * dots::container<T>() of the global DOTS API are used (see dots.h),
         * establishing the connection will include preloading of each type
         * used to instantiate those templates.
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
         * @param name The name that will be used by the global Transceiver to
         * identify itself.
         *
         * @param argc The number of command line arguments as given in the
         * main() function of the application.
         *
         * @param argv The command line arguments as given in the main()
         * function of the application.
         *
         * @exception std::exception Thrown if no connection could be
         * established based on the given arguments.
         */
        Application(const std::string& name, int& argc, char* argv[]);

        /*!
         * @brief Destroy the Application object.
         *
         * If the application is still running, this will stop the event loop
         * as if Application::exit() were called.
         */
        virtual ~Application();

        /*!
         * @brief Execute the application.
         *
         * This will block and run the global event loop (i.e. run the global
         * IO context) until the Application has exited (see
         * Application::exit()) or all work has finished.
         *
         * @return int The exit code of the application as passed in
         * Application::exit().
         */
        virtual int exec();

        /*!
         * @brief Execute at most one handler of the application until a
         * specific timeout.
         *
         * This function is similar to Application::exec() except that it will
         * only at most execute one ready handler before returning.
         *
         * If no handler is ready when this function is called, the function
         * will block and wait at most for the given amount of time.
         *
         * @param timeout The maximum amount of time to block and wait for one
         * handler to become ready.
         *
         * @return int The exit code of the application as passed in
         * Application::exit().
         */
        virtual int execOne(const std::chrono::milliseconds& timeout);

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
        virtual void exit(int exitCode = 0);

        /*!
         * @brief Get the most recent Application instance.
         *
         * Note that this is not a singleton accessor. Instead, it provides a
         * pointer to the most recently constructed Application instance.
         *
         * @return Application* A pointer to most recently Application
         * instance. Will be nullptr if no Application was yet constructed.
         */
        static Application* instance();

    private:

        void parseProgramOptions(int argc, char* argv[]);

        inline static Application* m_instance = nullptr;
        int m_exitCode;
        std::optional<io::Endpoint> m_openEndpoint;
    };
}
