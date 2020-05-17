#pragma once
#include <string_view>
#include <optional>
#include <dots/io/Transceiver.h>
#include <dots/io/Connection.h>

namespace dots
{
	struct GuestTransceiver : Transceiver
	{
		GuestTransceiver(std::string selfName);
		GuestTransceiver(const GuestTransceiver& other) = delete;
		GuestTransceiver(GuestTransceiver&& other) = default;
		virtual ~GuestTransceiver() = default;

		GuestTransceiver& operator = (const GuestTransceiver& rhs) = delete;
		GuestTransceiver& operator = (GuestTransceiver&& rhs) = default;

		const io::Connection& open(channel_ptr_t channel, descriptor_map_t preloadPublishTypes = {}, descriptor_map_t preloadSubscribeTypes = {});
		void publish(const type::Struct& instance, types::property_set_t what = types::property_set_t::All, bool remove = false) override;

	private:

        void joinGroup(const std::string_view& name) override;
		void leaveGroup(const std::string_view& name) override;

		bool handleReceive(io::Connection& connection, Transmission transmission, bool isFromMyself);
		void handleTransition(io::Connection& connection, const std::exception_ptr& e) noexcept;

		std::optional<io::Connection> m_hostConnection;
		descriptor_map_t m_preloadPublishTypes;
		descriptor_map_t m_preloadSubscribeTypes;
	};
}