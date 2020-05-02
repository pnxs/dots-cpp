#pragma once
#include <string>
#include <system_error>
#include <type_traits>
#include <dots/io/services/Transmission.h>
#include <DotsTransportHeader.dots.h>

namespace dots::io
{
	struct Registry;
}

namespace dots
{
    namespace tools
    {
		template <typename Derived>
		struct key
		{
            key(const key& other) = default;
            key(key&& other) = default;
            ~key() = default;

            key& operator = (const key& rhs) = default;
            key& operator = (key&& rhs) = default;

		private:

			friend Derived;
			key() = default;
		};

        struct shared_ptr_only
        {
			using key_t = key<shared_ptr_only>;

            shared_ptr_only(key_t) {};
            shared_ptr_only(const shared_ptr_only& other) = delete;
            shared_ptr_only(shared_ptr_only&& other) = delete;
            ~shared_ptr_only() = default;

            shared_ptr_only& operator = (const shared_ptr_only& rhs) = delete;
            shared_ptr_only& operator = (shared_ptr_only&& rhs) = delete;

		private:

			template <typename T, typename... Args>
            friend std::shared_ptr<T> make_shared_ptr_only(Args&&... args);

		    static constexpr key_t Key{};
        };

		template <typename T, typename... Args>
	    std::shared_ptr<T> make_shared_ptr_only(Args&&... args)
	    {
	        return std::make_shared<T>(shared_ptr_only::Key, std::forward<Args>(args)...);
	    }
    }

	struct Channel : tools::shared_ptr_only, std::enable_shared_from_this<Channel>
	{
		using key_t = tools::shared_ptr_only::key_t;
		using receive_handler_t = std::function<bool(const DotsTransportHeader&, Transmission&&)>;
		using error_handler_t = std::function<void(const std::exception_ptr&)>;

		Channel(key_t key);
		Channel(const Channel& other) = delete;
		Channel(Channel&& other) = delete;
		virtual ~Channel() noexcept = default;

		Channel& operator = (const Channel& rhs) = delete;
		Channel& operator = (Channel&& rhs) = delete;

		void init(io::Registry& registry);

		void asyncReceive(receive_handler_t&& receiveHandler, error_handler_t&& errorHandler);
		void transmit(const DotsTransportHeader& header, const type::Struct& instance);
		void transmit(const DotsTransportHeader& header, const Transmission& transmission);

	protected:

		const io::Registry& registry() const;
		io::Registry& registry();

		virtual void asyncReceiveImpl() = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const type::Struct& instance) = 0;
		virtual void transmitImpl(const DotsTransportHeader& header, const Transmission& transmission);

		void processReceive(const DotsTransportHeader& header, Transmission&& transmission) noexcept;
		void processError(const std::exception_ptr& e);
		void processError(const std::string& what);
		void verifyErrorCode(const std::error_code& errorCode);

	private:

		template <typename T, typename... Args>
        friend std::shared_ptr<T> make_channel(Args&&... args);

		void verifyInitialized() const;

		bool m_initialized;
		io::Registry* m_registry;
		receive_handler_t m_receiveHandler;
		error_handler_t m_errorHandler;
	};

	using channel_ptr_t = std::shared_ptr<Channel>;

	template <typename T, typename... Args>
	std::shared_ptr<T> make_channel(Args&&... args)
	{
		static_assert(std::is_base_of_v<Channel, T>, "T must be derived from Channel");
	    return tools::make_shared_ptr_only<T>(std::forward<Args>(args)...);
	}
}