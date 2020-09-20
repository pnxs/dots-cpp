#pragma once
#include <atomic>
#include <memory>
#include <dots/type/StructDescriptor.h>

namespace dots::io
{
    struct Dispatcher;

    struct [[nodiscard]] Subscription
    {
        using id_t = uint64_t;

        Subscription(std::weak_ptr<Dispatcher*> dispatcher, const type::StructDescriptor<>& descriptor);
        Subscription(const Subscription& other) = delete;
        Subscription(Subscription&& other) noexcept;
        ~Subscription();

        Subscription& operator = (const Subscription& rhs) = delete;
        Subscription& operator = (Subscription&& rhs) noexcept;

        const type::StructDescriptor<>& descriptor() const;
        id_t id() const;
        void unsubscribe();

        void discard();

    private:

        inline static std::atomic<id_t> M_lastId = 0;
        std::weak_ptr<Dispatcher*> m_dispatcher;
        const type::StructDescriptor<>* m_descriptor;
        id_t m_id;
    };
}