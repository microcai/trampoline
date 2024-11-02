
#pragma once

#include <memory>

namespace dr
{
    template<typename T>
    struct default_allocator;

    template <typename Signature>
    struct base_function;

    template <typename R, typename... Args>
    struct base_function<R(Args...)>
    {
        virtual ~base_function(){}
        virtual R call(Args...) = 0;
        virtual R call(Args...) const = 0;
    };

    template <typename Signature, typename Allocator = std::allocator<base_function<Signature>> >
    struct function;

    template<typename T>
    struct default_allocator
    {
        using allocator = std::allocator<typename function<T>::base_function >;
    };

    template <typename Allocator, typename R, typename... Args>
    struct function<R(Args...), Allocator>
    {
        using base_function = dr::base_function<R(Args...)>;

        Allocator m_allocator;
        base_function * impl;
        int impl_size;

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
            return impl != nullptr;
        }

        ~function()
        {
            if (impl)
            {
                impl->~base_function();
                m_allocator.deallocate(impl, impl_size);
            }
        }

        function()
            : impl(nullptr)
        {}

        template<typename Callable> requires std::is_invocable_r_v<R, Callable, Args...>
        function(Callable&& callable, Allocator allocator = {})
            : m_allocator(allocator)
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

            auto obj_size = sizeof(real_impl);
            auto new_stor = reinterpret_cast<void*>( m_allocator.allocate(obj_size) );
            impl = new (new_stor) real_impl{std::forward<Callable>(callable)};
        }
    };

}
