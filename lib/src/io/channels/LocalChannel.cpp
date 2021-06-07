#include <dots/io/channels/LocalChannel.h>
#include <dots/serialization/CborSerializer.h>
#include <dots/type/Registry.h>
#include <dots/io/channels/LocalListener.h>

namespace dots::io
{
    LocalChannel::LocalChannel(Channel::key_t key, boost::asio::io_context& ioContext) :
        Channel(key),
        m_ioContext{ std::ref(ioContext) }
    {
        initEndpoints(Endpoint{ "local:/" }, Endpoint{ "local:/" });
    }

    LocalChannel::LocalChannel(Channel::key_t key, boost::asio::io_context& ioContext, LocalListener& peer) :
        LocalChannel(key, ioContext)
    {
        peer.accept(*this);
    }

    void LocalChannel::link(LocalChannel& other)
    {
        if (!m_peer.expired())
        {
            throw std::logic_error{ "local channel can only be linked to one other channel at a time" };
        }

        m_peer = std::static_pointer_cast<LocalChannel>(other.shared_from_this());
    }

    void LocalChannel::asyncReceiveImpl()
    {
        /* do nothing */
    }

    void LocalChannel::transmitImpl(const DotsHeader& header, const type::Struct& instance)
    {
        auto other = m_peer.lock();

        if (other == nullptr)
        {
            throw std::runtime_error{ "local channel is not linked or expired unexpectedly" };
        }

        boost::asio::post(other->m_ioContext.get(), [this, this_{ weak_from_this() }, header = header, instance = type::AnyStruct{ instance }]() mutable
        {
            try
            {
                if (this_.expired())
                {
                    return;
                }

                auto other = m_peer.lock();

                if (other == nullptr)
                {
                    throw std::runtime_error{ "linked local channel expired unexpectedly" };
                }

                const type::StructDescriptor<>* descriptor = other->registry().findStructType(*header.typeName);

                if (descriptor == nullptr)
                {
                    throw std::runtime_error{ "encountered unknown type: " + *header.typeName };
                }

                // note: a serialization roundtrip is performed here to ensure that the descriptor of the local registry is used. this is
                // required, because descriptors are not guaranteed to be identical (e.g. when mixing static and dynamic descriptors).
                type::AnyStruct instance_{ *descriptor };
                std::vector<uint8_t> data = to_cbor(*instance, *header.attributes);
                from_cbor(data, instance_.get());

                other->processReceive(Transmission{ std::move(header), std::move(instance_) });
            }
            catch (...)
            {
                processError(std::current_exception());
            }
        });
    }
}