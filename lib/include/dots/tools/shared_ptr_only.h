#pragma once
#include <memory>

namespace dots::tools
{
    template <typename Derived>
    struct key
    {
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
        virtual ~shared_ptr_only() = default;

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
