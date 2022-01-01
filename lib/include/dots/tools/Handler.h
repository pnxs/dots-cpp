#pragma once
#include <type_traits>
#include <functional>

namespace dots::tools
{
    template <typename R, typename... Args>
    struct HandlerBase
    {
        using return_t = R;
        using args_t = std::tuple<Args...>;

        template <typename Invocable>
        using is_compatible_invocable = std::conjunction<
            std::negation<std::is_same<std::decay_t<Invocable>, HandlerBase>>,
            std::is_invocable_r<R, std::decay_t<Invocable>, Args...>
        >;

        template <typename Invocable, typename... BindArgs>
        using is_const_compatible_invocable = std::conjunction<
            std::negation<std::is_member_function_pointer<std::decay_t<Invocable>>>,
            std::is_invocable_r<R, const std::decay_t<Invocable>, const std::decay_t<BindArgs>&..., Args...>
        >;
        
        template <typename Invocable, typename... BindArgs>
        using is_mutable_compatible_invocable = std::conjunction<
            std::negation<std::is_member_function_pointer<std::decay_t<Invocable>>>,
            std::is_invocable_r<R, std::decay_t<Invocable>, std::decay_t<BindArgs>&..., Args...>
        >;

        template <typename MemFn, typename Obj, typename... BindArgs>
        using is_const_compatible_member_function = std::conjunction<
            std::is_member_function_pointer<std::decay_t<MemFn>>,
            std::is_pointer<std::decay_t<Obj>>,
            std::is_invocable_r<R, const std::decay_t<MemFn>, const std::remove_pointer_t<std::decay_t<Obj>>*, const std::decay_t<BindArgs>&..., Args...>
        >;

        template <typename MemFn, typename Obj, typename... BindArgs>
        using is_mutable_compatible_member_function = std::conjunction<
            std::is_member_function_pointer<std::decay_t<MemFn>>,
            std::is_pointer<std::decay_t<Obj>>,
            std::negation<std::is_invocable_r<R, std::decay_t<MemFn>, const std::remove_pointer_t<std::decay_t<Obj>>*, const BindArgs&..., Args...>>,
            std::is_invocable_r<R, std::decay_t<MemFn>, std::remove_pointer_t<std::decay_t<Obj>>*, std::decay_t<BindArgs>&..., Args...>
        >;

        template <typename Invocable, std::enable_if_t<is_compatible_invocable<Invocable>::value, int> = 0>
        HandlerBase(Invocable&& invocable) :
            m_handler{ std::forward<Invocable>(invocable) }
        {
            /* do nothing */
        }

        template <typename Invocable, typename... BindArgs, std::enable_if_t<is_const_compatible_invocable<Invocable, BindArgs...>::value, int> = 0>
        HandlerBase(Invocable&& invocable, BindArgs&&... bindArgs) :
            m_handler{ wrapConstInvocable(std::forward<Invocable>(invocable), std::forward<BindArgs>(bindArgs)...) }
        {
            /* do nothing */
        }

        template <typename Invocable, typename... BindArgs, std::enable_if_t<is_mutable_compatible_invocable<Invocable, BindArgs...>::value, int> = 0>
        HandlerBase(Invocable&& invocable, BindArgs&&... bindArgs) :
            m_handler{ wrapMutableInvocable(std::forward<Invocable>(invocable), std::forward<BindArgs>(bindArgs)...) }
        {
            /* do nothing */
        }

        template <typename MemFn, typename Obj, typename... BindArgs, std::enable_if_t<is_const_compatible_member_function<MemFn, Obj, BindArgs...>::value, int> = 0>
        HandlerBase(MemFn&& memFn, Obj&& obj, BindArgs&&... bindArgs) :
            m_handler{ wrapConstInvocable(std::forward<MemFn>(memFn), std::forward<Obj>(obj), std::forward<BindArgs>(bindArgs)...) }
        {
            /* do nothing */
        }

        template <typename MemFn, typename Obj, typename... BindArgs, std::enable_if_t<is_mutable_compatible_member_function<MemFn, Obj, BindArgs...>::value, int> = 0>
        HandlerBase(MemFn&& memFn, Obj&& obj, BindArgs&&... bindArgs) :
            m_handler{ wrapMutableInvocable(std::forward<MemFn>(memFn), std::forward<Obj>(obj), std::forward<BindArgs>(bindArgs)...) }
        {
            /* do nothing */
        }

        HandlerBase(const HandlerBase& other) = default;
        HandlerBase(HandlerBase&& other) = default;
        ~HandlerBase() = default;

        HandlerBase& operator = (const HandlerBase& rhs) = default;
        HandlerBase& operator = (HandlerBase&& rhs) = default;

        R operator () (Args... args) const
        {
            return m_handler(std::forward<Args>(args)...);
        }

    protected:

        HandlerBase() = default;

        std::function<R(Args...)> m_handler;

    private:

        template <typename Invocable, typename... BindArgs>
        auto wrapConstInvocable(Invocable&& invocable, BindArgs&&... bindArgs)
        {
            return [invocable{ std::forward<Invocable>(invocable) }, bindTuple = std::make_tuple(std::forward<BindArgs>(bindArgs)...)](Args... args) -> R
            {
                return std::apply([&](auto&&... bindArgs)
                {
                    return std::invoke(invocable, std::forward<decltype(bindArgs)>(bindArgs)..., std::forward<Args>(args)...);
                }, bindTuple);
            };
        }

        template <typename Invocable, typename... BindArgs>
        auto wrapMutableInvocable(Invocable&& invocable, BindArgs&&... bindArgs)
        {
            return [invocable{ std::forward<Invocable>(invocable) }, bindTuple = std::make_tuple(std::forward<BindArgs>(bindArgs)...)](Args... args) mutable -> R
            {
                return std::apply([&](auto&&... bindArgs)
                {
                    return std::invoke(invocable, std::forward<decltype(bindArgs)>(bindArgs)..., std::forward<Args>(args)...);
                }, bindTuple);
            };
        }
    };

    template <typename T>
    struct Handler
    {
        static_assert(std::negation_v<std::is_same<T, T>>, "T has to be a function type");
    };

    template <typename R>
    struct Handler<R()> : HandlerBase<R>
    {
        using HandlerBase<R>::HandlerBase;
    };

    struct static_argument_cast_tag{};
    constexpr static_argument_cast_tag static_argument_cast;

    template <typename R, typename Arg>
    struct Handler<R(Arg)> : HandlerBase<R, Arg>
    {
        template <typename ArgOther>
        using is_compatible_by_argument = std::conjunction<
            std::negation<std::is_same<std::decay_t<Handler<R(ArgOther)>>, Handler>>,
            std::is_base_of<std::decay_t<Arg>, std::decay_t<ArgOther>>
        >;

        using HandlerBase<R, Arg>::HandlerBase;

        template <typename ArgOther, std::enable_if_t<is_compatible_by_argument<ArgOther>::value, int> = 0>
        Handler(static_argument_cast_tag, Handler<R(ArgOther)>&& other)
        {
            HandlerBase<R, Arg>::m_handler = [handler{ std::move(other.m_handler)}](Arg arg) -> R
            {
                return std::invoke(handler, static_cast<ArgOther>(arg));
            };
        }

    private:

        template <typename>
        friend struct Handler;
    };

    template <typename R, typename... Args>
    struct Handler<R(Args...)> : HandlerBase<R, Args...>
    {
        using HandlerBase<R, Args...>::HandlerBase;
    };

    namespace details
    {
        template <typename T, typename = void>
        struct deduce_signature
        {
            static_assert(std::negation_v<std::is_same<T, T>>, "T has to be a function type");
        };

        template <typename R, typename T, typename... Args>
        struct deduce_signature<R (T::*)(Args...)>
        {
            using type = R(Args...);
        };

        template <typename R, typename T, typename... Args>
        struct deduce_signature<R (T::*)(Args...) const>
        {
            using type = R(Args...);
        };

        template <typename T>
        struct deduce_signature<T, std::void_t<decltype(&T::operator())>> : deduce_signature<decltype(&T::operator())>
        {
        };

        template <typename T>
        using deduce_signature_t = typename deduce_signature<T>::type;
    }

    template <typename Invocable, typename... BindArgs>
    Handler(Invocable&&, BindArgs&&... bindArgs) -> Handler<details::deduce_signature_t<Invocable>>;

    template <typename T>
    struct is_handler : std::false_type {};

    template <typename T>
    struct is_handler<Handler<T>> : std::true_type {};

    template <typename T>
    using is_handler_t = typename is_handler<T>::type;

    template <typename T>
    constexpr bool is_handler_v = is_handler<T>::value;
}
