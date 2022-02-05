#pragma once
#include <string_view>
#include <optional>
#include <set>
#include <dots/type/DescriptorMap.h>
#include <dots/Transceiver.h>
#include <dots/Connection.h>

namespace dots
{
    /*!
     * @class GuestTransceiver GuestTransceiver.h <dots/GuestTransceiver.h>
     *
     * @brief Transceiver for attending DOTS spaces as a guest.
     *
     * The GuestTransceiver class implements the "DOTS guest" concept.
     * Guests attend "DOTS spaces" that are created by "DOTS hosts".
     *
     * A space is an environment in which attendees (including the host)
     * can subscribe to DOTS struct types and publish instances of types
     * into.
     *
     * Even though a GuestTransceiver often technically acts as a client,
     * it is agnostic about how a connection is established. A
     * GuestTransceiver can asynchronously open a host connection from
     * given io::Channel instances.
     *
     * After the host connection has been established, the GuestTransceiver
     * can be used to subscribe to DOTS struct types, as well as publish
     * instances into the attended space.
     */
    struct GuestTransceiver : Transceiver
    {
        /*!
         * @brief Construct a new GuestTransceiver object.
         *
         * After construction, the transceiver will be inactive until a
         * connection is created via GuestTransceiver::open().
         *
         * @param selfName The name the transceiver will use to identify
         * itself.
         *
         * @param ioContext The ASIO IO context (i.e. the "event loop") to use.
         *
         * @param staticTypePolicy Specifies the static type policy of the
         * transceiver's registry.
         */
        GuestTransceiver(std::string selfName, asio::io_context& ioContext = io::global_io_context(), type::Registry::StaticTypePolicy staticTypePolicy = type::Registry::StaticTypePolicy::All);
        GuestTransceiver(const GuestTransceiver& other) = delete;
        GuestTransceiver(GuestTransceiver&& other) = default;
        ~GuestTransceiver() override = default;

        GuestTransceiver& operator = (const GuestTransceiver& rhs) = delete;
        GuestTransceiver& operator = (GuestTransceiver&& rhs) = default;

        /*!
         * @brief Get current host connection.
         *
         * @warning The state of the std::optional must always be checked
         * before accessing the contained object, as the Connection object will
         * be destroyed when the connection is closed.
         *
         * @return const std::optional<Connection>& A reference to the current
         * host connection. Might be empty if no connection was opened by
         * GuestTransceiver::open() or if it already has been closed.
         */
        const std::optional<Connection>& connection() const;

        /*!
         * @brief Start to asynchronously open and establish a host connection
         * via a specific channel.
         *
         * @param preloadPublishTypes The publish types to preload.
         *
         * @param preloadSubscribeTypes The subscribe types to preload.
         *
         * @param authSecret The secret to use for authentication if requested
         * by the host.
         *
         * @param channel The channel to use to asynchronously open and
         * establish a host connection.
         *
         * @return const Connection& A reference to the host connection after
         * asynchronous receiving has started.
         *
         * @exception std::logic_error Thrown if another host connection has
         * already been opened.
         */
        const Connection& open(type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret, io::channel_ptr_t channel);

        /*!
         * @brief Asynchronously open and establish a host connection via a
         * specific channel.
         *
         * @param channel The channel to use to asynchronously open and
         * establish a host connection.
         *
         * @return const Connection& A reference to the host connection after
         * asynchronous receiving has started.
         *
         * @exception std::logic_error Thrown if another host connection has
         * already been opened.
         */
        const Connection& open(io::channel_ptr_t channel);

        /*!
         * @brief Construct a specific channel and use it to asynchronously
         * open and establish a host connection.
         *
         * @tparam TChannel The type of the channel to construct.
         *
         * @tparam Args The types of the arguments to forward to the compatible
         * constructor of @p TChannel .
         *
         * @param preloadPublishTypes The publish types to preload.
         *
         * @param preloadSubscribeTypes The subscribe types to preload.
         *
         * @param authSecret The secret to use for authentication if requested
         * by the host.
         *
         * @param args The arguments to forward to the compatible constructor
         * of @p TChannel .
         *
         * @return const Connection& A reference to the host connection after
         * asynchronous receiving has started.
         *
         * @exception std::logic_error Thrown if another host connection has
         * already been opened.
         */
        template <typename TChannel, typename... Args>
        const Connection& open(type::DescriptorMap preloadPublishTypes, type::DescriptorMap preloadSubscribeTypes, std::optional<std::string> authSecret, Args&&... args)
        {
            return open(std::move(preloadPublishTypes), std::move(preloadSubscribeTypes), std::move(authSecret), io::make_channel<TChannel>(ioContext(), std::forward<Args>(args)...));
        }

        /*!
         * @brief Construct a specific channel and use it to asynchronously
         * open and establish a host connection.
         *
         * @tparam TChannel The type of the channel to construct.
         *
         * @tparam Args The types of the arguments to forward to the compatible
         * constructor of @p TChannel .
         *
         * @param args The arguments to forward to the compatible constructor
         * of @p TChannel .
         *
         * @return const Connection& A reference to the host connection after
         * asynchronous receiving has started.
         *
         * @exception std::logic_error Thrown if another host connection has
         * already been opened.
         */
        template <typename TChannel, typename... Args>
        const Connection& open(Args&&... args)
        {
            return open(io::make_channel<TChannel>(ioContext(), std::forward<Args>(args)...));
        }

        /*!
         * @brief Publish an instance of a DOTS struct type.
         *
         * This will create a corresponding io::Transmission for the publish
         * and asynchronously transmit it via the current host connection.
         *
         * Note that this will neither directly invoke any callbacks of local
         * subscribers nor have any effect on the local container. If the
         * transceiver itself has a subscription to the instance type, the
         * instance will be dispatched once it is acknowledged (i.e. sent back)
         * by the host.
         *
         * @param instance The instance to publish.
         *
         * @param includedProperties The properties to publish in addition to
         * the key properties. If no set is given, the valid property set of
         * @p instance will be used.
         *
         * @param remove Specifies whether the publish is a remove.
         *
         * @exception std::logic_error Thrown if @p instance is of a
         * 'substruct-only' type.
         *
         * @exception std::runtime_error Thrown if a key property of the
         * instance is invalid.
         *
         * @exception std::runtime_error Thrown if no host connection has been
         * established when the function is called.
         */
        void publish(const type::Struct& instance, std::optional<property_set_t> includedProperties = std::nullopt, bool remove = false) override;

    private:

        void joinGroup(std::string_view name) override;
        void leaveGroup(std::string_view name) override;

        bool handleTransmission(Connection& connection, io::Transmission transmission);
        void handleTransition(Connection& connection, std::exception_ptr ePtr) noexcept;

        std::optional<Connection> m_hostConnection;
        type::DescriptorMap m_preloadPublishTypes;
        type::DescriptorMap m_preloadSubscribeTypes;
        std::set<std::string> m_joinedGroups;
    };
}
