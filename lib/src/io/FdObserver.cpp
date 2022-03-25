#include <dots/asio.h>
#if defined(BOOST_ASIO_HAS_POSIX_STREAM_DESCRIPTOR)
#include <dots/io/FdObserver.h>

namespace dots::io::posix
{
    using stream_descriptor_t = asio::posix::stream_descriptor;

    struct FdObserver::observer_data
    {
        stream_descriptor_t streamDescriptor;
        handler_t handler;
        stream_descriptor_t::wait_type waitType;
        bool discarded;
    };

    static stream_descriptor_t::wait_type condition_to_wait_type(FdObserver::Condition condition)
    {
        switch (condition)
        {
            case FdObserver::Condition::ReadyForRead:
                return stream_descriptor_t::wait_read;
            case FdObserver::Condition::ReadyForWrite:
                return stream_descriptor_t::wait_write;
            case FdObserver::Condition::Error:
                return stream_descriptor_t::wait_error;
        }

        throw std::runtime_error{ "invalid condition" };
    }

    FdObserver::FdObserver(asio::io_context& ioContext, int fileDescriptor, handler_t handler, Condition condition/* = Condition::ReadyForRead*/) :
        m_observerData{ std::make_shared<observer_data>(observer_data{
            stream_descriptor_t{ ioContext, fileDescriptor },
            std::move(handler),
            condition_to_wait_type(condition),
            false
        } ) }
    {
        AsyncWait(m_observerData);
    }

    FdObserver::~FdObserver()
    {
        try
        {
            if (m_observerData != nullptr)
            {
                m_observerData->streamDescriptor.cancel();
            }
        }
        catch (...)
        {
            /* do nothing */
        }
    }

    void FdObserver::discard()
    {
        if (m_observerData != nullptr)
        {
            m_observerData->discarded = true;
            m_observerData = nullptr;
        }
    }

    void FdObserver::AsyncWait(const std::shared_ptr<observer_data>& observerData)
    {
        observerData->streamDescriptor.async_wait(observerData->waitType, [observerData](boost::system::error_code error)
        {
            if ((observerData.use_count() == 1 && !observerData->discarded) || error == asio::error::operation_aborted)
            {
                return;
            }

            if (error)
            {
                observerData->handler(std::make_exception_ptr(std::runtime_error{ error.message() }));
            }
            else
            {
                observerData->handler(nullptr);
                AsyncWait(observerData);
            }
        });
    }
}
#endif
