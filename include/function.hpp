
#pragma once

#include <memory>

namespace dr
{
    template <typename Signature, typename Allocator = std::allocator<char>>
    struct function;

    template <typename Allocator, typename R, typename... Args>
    struct function<R(Args...), Allocator>
    {
        Allocator allocator;
        struct base_function
        {
            virtual ~base_function(){}
            virtual R call(Args...) = 0;
            virtual R call(Args...) const = 0;
        };

        std::unique_ptr<base_function> impl;

        R operator()(Args... args) const
        {
            return impl->call(args...);
        }

        R operator()(Args... args)
        {
            return impl->call(args...);
        }

        operator bool() const
        {
            return impl.get() != nullptr;
        }

        function()
        {}

        template<typename Callable> requires std::is_invocable_r_v<R, Callable, Args...>
        function(Callable&& callable)
        {
            struct real_impl : public base_function
            {
                Callable m_callable;

                virtual R call(Args... args) const override
                {
                    return m_callable(args...);
                }

                virtual R call(Args... args) override
                {
                    return m_callable(args...);
                }

                real_impl(Callable && callable)
                    : m_callable(std::forward<Callable>(callable))
                {}
            };

            auto new_stor = reinterpret_cast<void*>( allocator.allocate(sizeof(real_impl)) );
            impl.reset( new (new_stor) real_impl{std::forward<Callable>(callable)} );
        }
    };

}
