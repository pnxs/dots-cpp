#pragma once
#include <exception>
#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
#include <dots/tools/Handler.h>

namespace dots::io::posix
{
    /*!
     * @class FdObserver FdObserver.h <dots/io/FdObserver.h>
     *
     * @brief Scoped resource for file descriptor observations.
     *
     * An object of this class is a RAII-style resource that represents an
     * observation of a file descriptor, such as obtained by system calls
     * like ::open().
     *
     * The file descriptor can be associated with any file that supports
     * asynchronous events on the current platform.
     *
     * This usually includes devices, sockets and pipes, but not regular
     * files.
     *
     * Note that FdObserver objects can safely be destroyed prematurely, in
     * which case the observation will be cancelled without invoking the
     * handler.
     */
    struct FdObserver
    {
        using handler_t = tools::Handler<void(std::exception_ptr)>;

        enum struct Condition
        {
            ReadyForRead,
            ReadyForWrite,
            Error
        };

        /*!
         * @brief Construct a new FdObserver object.
         *
         * The file associated with the given file descriptor will be observed
         * asynchronously within the given IO context and the given handler
         * invoked whenever the given observe condition is reached.
         *
         * @param ioContext The IO context (i.e. event loop) to associate with
         * the observation.
         *
         * @param fileDescriptor The descriptor associated with the file to
         * observe asynchronously.
         *
         * @param handler The handler to invoke asynchronously every time the
         * file has reached the given condition.
         *
         * @param condition The observe condition to wait for.
         */
        FdObserver(asio::io_context& ioContext, int fileDescriptor, handler_t handler, Condition condition = Condition::ReadyForRead);

        FdObserver(const FdObserver& other) = delete;
        FdObserver(FdObserver&& other) = default;

        /*!
         * @brief Destroy the FdObserver object.
         *
         * When the FdObserver object is destroyed and is observing a file
         * (i.e. was not discarded), the observation will safely be cancelled
         * without invoking the handler given in FdObserver().
         */
        ~FdObserver();

        FdObserver& operator = (const FdObserver& rhs) = delete;
        FdObserver& operator = (FdObserver&& rhs) = default;

        /*!
         * @brief Release management of the FdObserver.
         *
         * Calling this function will decouple the observation from the
         * FdObserver object's lifetime, without invoking the handler. As a
         * result, this FdObserver object will be empty when the function
         * returns.
         *
         * Note that this will have no effect if the FdObserver object is
         * already empty when the function is called.
         *
         * @warning Calling this function will make it impossible to manually
         * cancel the observation.
         *
         * @remark This function is intended to cover simple use cases where
         * the durations of observations are bound to an application's lifetime
         * and management is not required.
         */
        void discard();

    private:

        struct observer_data;
        static void AsyncWait(const std::shared_ptr<observer_data>& m_observerData);
        std::shared_ptr<observer_data> m_observerData;
    };
}
#else
#error "The FdObserver requires support for POSIX stream descriptors and is not available on this platform"
#endif
