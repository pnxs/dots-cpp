#pragma once
#include <type_traits>
#include <functional>

namespace dots::tools
{
    namespace details
    {
        /*!
         * @class HandlerBase Handler.h <dots/tools/Handler.h>
         *
         * @brief Base class for all universal handlers.
         *
         * @warning This base class is an implementation detail and should not
         * be used directly.
         */
        template <typename R, typename... Args>
        struct HandlerBase
        {
            using return_t = R;
            using args_t = std::tuple<Args...>;

            template <typename Invocable>
            static constexpr bool is_compatible_v = std::conjunction_v<
                std::negation<std::is_same<std::decay_t<Invocable>, HandlerBase>>,
                std::is_invocable_r<R, std::decay_t<Invocable>, Args...>
            >;

            template <typename MemFn, typename Obj>
            static constexpr bool is_compatible_member_function_v = std::conjunction_v<
                std::is_member_function_pointer<std::decay_t<MemFn>>,
                std::is_pointer<std::decay_t<Obj>>,
                std::is_invocable_r<R, std::decay_t<MemFn>, std::decay_t<Obj>, Args...>
            >;
            
            template <typename Invocable, typename... BindArgs>
            static constexpr bool is_bind_compatible_v = std::conjunction_v<
                std::negation<std::is_member_function_pointer<std::decay_t<Invocable>>>,
                std::is_invocable_r<R, std::decay_t<Invocable>, std::decay_t<BindArgs>&..., Args...>
            >;

            template <typename MemFn, typename Obj, typename... BindArgs>
            static constexpr bool is_bind_compatible_member_function_v = std::conjunction_v<
                std::is_member_function_pointer<std::decay_t<MemFn>>,
                std::is_pointer<std::decay_t<Obj>>,
                std::is_invocable_r<R, std::decay_t<MemFn>, std::decay_t<Obj>, std::decay_t<BindArgs>&..., Args...>
            >;

            /*!
             * @brief Construct a new Handler from a specific invocable object.
             *
             * @tparam Invocable The type of the invocable object to use as a
             * handler.
             *
             * @param invocable The object to use as a handler. This can be any
             * invocable object (e.g a lambda, a global function or a static member
             * function) that is compatible with the signature \p  R(Args...) .
             */
            template <typename Invocable, std::enable_if_t<is_compatible_v<Invocable>, int> = 0>
            HandlerBase(Invocable&& invocable) :
                m_handler{ std::forward<Invocable>(invocable) }
            {
                /* do nothing */
            }

            /*!
             * @brief Construct a new Handler from a specific member function.
             *
             * @tparam MemFn The type of the member function to use as a handler.
             *
             * @tparam Obj The class of the member function.
             *
             * @param memFn A pointer to the member function to use as a handler.
             * This can be any function that is compatible with the signature
             * \p R(Args...) .
             *
             * @param obj The object to invoke the member function on.
             */
            template <typename MemFn, typename Obj, std::enable_if_t<is_compatible_member_function_v<MemFn, Obj>, int> = 0>
            HandlerBase(MemFn&& memFn, Obj&& obj) :
                m_handler{ wrapInvocable(std::forward<MemFn>(memFn), std::forward<Obj>(obj)) }
            {
                /* do nothing */
            }

            /*!
             * @brief Construct a new Handler from a specific invocable object with
             * bound arguments.
             *
             * @tparam Invocable The type of the invocable object to use as a
             * handler.
             *
             * @tparam BindArgs The types of the arguments to bind to the handler.
             *
             * @param invocable The object to use as a handler. This can be any
             * invocable object (e.g a lambda, a global function or a static member
             * function) that is compatible with the signature
             * \p R(BindArgs&...,Args...) .
             *
             * @param bindArgs The arguments to bind and pass as references when
             * the handler is invoked.
             */
            template <typename Invocable, typename... BindArgs, std::enable_if_t<is_bind_compatible_v<Invocable, BindArgs...>, int> = 0>
            HandlerBase(Invocable&& invocable, BindArgs&&... bindArgs) :
                m_handler{ wrapInvocable(std::forward<Invocable>(invocable), std::forward<BindArgs>(bindArgs)...) }
            {
                /* do nothing */
            }

            /*!
             * @brief Construct a new Handler from a specific member function with
             * bound arguments.
             *
             * @tparam MemFn The type of the member function to use as a handler.
             *
             * @tparam Obj The class of the member function.
             *
             * @tparam BindArgs The types of the arguments to bind to the handler.
             *
             * @param memFn A pointer to the member function to use as a handler.
             * This can be any function that is compatible with the signature
             * \p R(BindArgs&...,Args...) .
             *
             * @param obj The object to invoke the member function on.
             *
             * @param bindArgs The arguments to bind and pass as references when
             * the handler is invoked.
             */
            template <typename MemFn, typename Obj, typename... BindArgs, std::enable_if_t<is_bind_compatible_member_function_v<MemFn, Obj, BindArgs...>, int> = 0>
            HandlerBase(MemFn&& memFn, Obj&& obj, BindArgs&&... bindArgs) :
                m_handler{ wrapInvocable(std::forward<MemFn>(memFn), std::forward<Obj>(obj), std::forward<BindArgs>(bindArgs)...) }
            {
                /* do nothing */
            }

            HandlerBase(const HandlerBase& other) = default;
            HandlerBase(HandlerBase&& other) = default;
            ~HandlerBase() = default;

            HandlerBase& operator = (const HandlerBase& rhs) = default;
            HandlerBase& operator = (HandlerBase&& rhs) = default;

            /*!
             * @brief Invoke the handler.
             *
             * This calls the underlying invocable object with the given arguments.
             *
             * @param args The arguments to call the invocable object with.
             *
             * @return R The return value of the call.
             */
            R operator () (Args... args) const
            {
                return m_handler(std::forward<Args>(args)...);
            }

        protected:

            HandlerBase() = default;

            std::function<R(Args...)> m_handler;

        private:

            template <typename Invocable, typename... BindArgs>
            using is_const_wrappable = std::is_invocable_r<R, std::decay_t<Invocable>, const std::decay_t<BindArgs>&..., Args...>;

            template <typename Invocable, typename... BindArgs>
            using is_mutable_wrappable = std::conjunction<
                std::negation<is_const_wrappable<Invocable, BindArgs...>>,
                std::is_invocable_r<R, std::decay_t<Invocable>, std::decay_t<BindArgs>&..., Args...>
            >;

            template <typename Invocable, typename... BindArgs, std::enable_if_t<is_const_wrappable<Invocable, BindArgs...>::value, int> = 0>
            auto wrapInvocable(Invocable&& invocable, BindArgs&&... bindArgs)
            {
                return [invocable{ std::forward<Invocable>(invocable) }, bindTuple = std::make_tuple(std::forward<BindArgs>(bindArgs)...)](Args... args) -> R
                {
                    return std::apply([&](auto&&... bindArgs)
                    {
                        return std::invoke(invocable, std::forward<decltype(bindArgs)>(bindArgs)..., std::forward<Args>(args)...);
                    }, bindTuple);
                };
            }

            template <typename Invocable, typename... BindArgs, std::enable_if_t<is_mutable_wrappable<Invocable, BindArgs...>::value, int> = 0>
            auto wrapInvocable(Invocable&& invocable, BindArgs&&... bindArgs)
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
    }

    template <typename T>
    struct Handler
    {
        static_assert(std::negation_v<std::is_same<T, T>>, "T has to be a function type");
    };

    /*!
     * @class Handler Handler.h <dots/tools/Handler.h>
     *
     * @brief Specialization for handlers without parameters.
     *
     * The Handler class can be used to store user-provided invocable
     * objects and use them as handlers.
     *
     * It provides similar functionality to std::function with additional
     * convenience functionality for member functions and argument binding.
     *
     * @remark Contrary to std::function, a Handler always contains a valid
     * invocable object and is therefore not default-constructible.
     * Instead, users are advised to use an std::optional when
     * "nullability" is required.
     */
    template <typename R>
    struct Handler<R()> : details::HandlerBase<R>
    {
        using details::HandlerBase<R>::HandlerBase;
    };

    struct static_argument_cast_tag{};
    constexpr static_argument_cast_tag static_argument_cast;

    /*!
     * @class Handler Handler.h <dots/tools/Handler.h>
     *
     * @brief Specialization for handlers with a single parameter.
     *
     * The Handler class can be used to store user-provided invocable
     * objects and use them as handlers.
     *
     * It provides similar functionality to std::function with additional
     * convenience functionality for member functions and argument binding.
     *
     * @remark Contrary to std::function, a Handler always contains a valid
     * invocable object and is therefore not default-constructible.
     * Instead, users are advised to use an std::optional when
     * "nullability" is required.
     */
    template <typename R, typename Arg>
    struct Handler<R(Arg)> : details::HandlerBase<R, Arg>
    {
        template <typename ArgOther>
        using is_compatible_by_argument = std::conjunction<
            std::negation<std::is_same<std::decay_t<Handler<R(ArgOther)>>, Handler>>,
            std::is_base_of<std::decay_t<Arg>, std::decay_t<ArgOther>>
        >;

        using details::HandlerBase<R, Arg>::HandlerBase;

        /*!
         * @brief Construct a new Handler by wrapping another compatible
         * handler and statically downcasting the argument.
         *
         * This constructs a Handler of signature \p R(Arg) from another
         * handler with signature \p R(ArgOther), where \p Arg is a base class
         * of \p ArgOther.
         *
         * When the handler is invoked, the argument will statically be
         * downcasted from \p Arg to \p ArgOther.
         *
         * @warning The implementation only enforces that type \p Arg is
         * statically downcastable to \p ArgOther. There are no runtime checks
         * that a particular argument is actually of that type. It is the
         * user's responsibility to ensure that the handler is only invoked
         * with appropriate arguments.
         *
         * @tparam ArgOther The argument to statically downcast to. Must be a
         * subclass of \p Arg .
         *
         * @param tag Disambiguation tag for static argument casts.
         *
         * @param other The handler object to wrap.
         */
        template <typename ArgOther, std::enable_if_t<is_compatible_by_argument<ArgOther>::value, int> = 0>
        Handler(static_argument_cast_tag tag, Handler<R(ArgOther)>&& other)
        {
            (void)tag;
            details::HandlerBase<R, Arg>::m_handler = [handler{ std::move(other.m_handler)}](Arg arg) -> R
            {
                return std::invoke(handler, static_cast<ArgOther>(arg));
            };
        }

    private:

        template <typename>
        friend struct Handler;
    };

    /*!
     * @class Handler Handler.h <dots/tools/Handler.h>
     *
     * @brief Specialization for handlers with on or more parameters.
     *
     * The Handler class can be used to store user-provided invocable
     * objects and use them as handlers.
     *
     * It provides similar functionality to std::function with additional
     * convenience functionality for member functions and argument binding.
     *
     * @remark Contrary to std::function, a Handler always contains a valid
     * invocable object and is therefore not default-constructible.
     * Instead, users are advised to use an std::optional when
     * "nullability" is required.
     */
    template <typename R, typename... Args>
    struct Handler<R(Args...)> : details::HandlerBase<R, Args...>
    {
        using details::HandlerBase<R, Args...>::HandlerBase;
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
