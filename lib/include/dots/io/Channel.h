#pragma once
#include <string>
#include <system_error>
#include <type_traits>
#include <set>
#include <unordered_set>
#include <dots/tools/Handler.h>
#include <dots/io/Endpoint.h>
#include <dots/io/Transmission.h>
#include <dots/tools/shared_ptr_only.h>
#include <DotsHeader.dots.h>

namespace dots::type
{
    struct Registry;
}

namespace dots::io
{
    struct Channel : tools::shared_ptr_only, std::enable_shared_from_this<Channel>
    {
        using key_t = key_t;
        using receive_handler_t = tools::Handler<bool(Transmission)>;
        using error_handler_t = tools::Handler<void(std::exception_ptr)>;

        Channel(key_t key);
        Channel(const Channel& other) = delete;
        Channel(Channel&& other) = delete;
        virtual ~Channel() noexcept = default;

        Channel& operator = (const Channel& rhs) = delete;
        Channel& operator = (Channel&& rhs) = delete;

        const Endpoint& localEndpoint();
        const Endpoint& remoteEndpoint();

        void init(type::Registry& registry);

        void asyncReceive(receive_handler_t receiveHandler, error_handler_t errorHandler);
        void transmit(const type::Struct& instance);
        void transmit(const DotsHeader& header, const type::Struct& instance);
        void transmit(const Transmission& transmission);
        void transmit(const type::Descriptor<>& descriptor);

    protected:

        void initEndpoints(Endpoint localEndpoint, Endpoint remoteEndpoint);

        const type::Registry& registry() const;
        type::Registry& registry();

        virtual void asyncReceiveImpl() = 0;
        virtual void transmitImpl(const DotsHeader& header, const type::Struct& instance) = 0;
        virtual void transmitImpl(const Transmission& transmission);

        void processReceive(Transmission transmission) noexcept;
        void processError(std::exception_ptr ePtr);
        void processError(const std::string& what);
        void verifyErrorCode(std::error_code errorCode);

    private:

        template <typename T, typename... Args>
        friend std::shared_ptr<T> make_channel(Args&&... args);

        void importDependencies(const type::Struct& instance);
        void exportDependencies(const type::Descriptor<>& descriptor);

        std::set<std::string> m_sharedTypeNames;
        std::unordered_set<const type::Descriptor<>*> m_sharedTypeDescriptors;
        bool m_asyncReceiving;
        bool m_initialized;
        type::Registry* m_registry;
        std::optional<Endpoint> m_localEndpoint;
        std::optional<Endpoint> m_remoteEndpoint;
        std::optional<receive_handler_t> m_receiveHandler;
        std::optional<error_handler_t> m_errorHandler;
    };

    using channel_ptr_t = std::shared_ptr<Channel>;

    template <typename T, typename... Args>
    std::shared_ptr<T> make_channel(Args&&... args)
    {
        static_assert(std::is_base_of_v<Channel, T>, "T must be derived from Channel");
        static_assert(std::is_constructible_v<T, tools::shared_ptr_only::key_t, Args...>, "channel T is not constructible from Args");

        return tools::make_shared_ptr_only<T>(std::forward<Args>(args)...);
    }
}
